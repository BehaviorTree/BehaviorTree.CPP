// bind_tree.cpp — Tree binding.
//
// tick_while_running uses the GIL-release pattern from the plan: the
// tick loop runs in pure C++ with the GIL dropped, so other Python threads
// can make progress. Between ticks we briefly reacquire to check signals
// (so Ctrl-C reaches the user within sleep_ms of being pressed).
//
// Live trees are tracked via weak_ptr so a Py_AtExit handler can halt any
// still-running tree at interpreter shutdown — prevents segfaults from a
// background trampoline reaching for a destroyed GIL.

#include <chrono>
#include <memory>
#include <mutex>
#include <vector>

#if defined(__unix__) || defined(__APPLE__)
#include <sys/types.h>
#include <unistd.h>
#endif

#include <nanobind/nanobind.h>
#include <nanobind/stl/shared_ptr.h>

#include "behaviortree_cpp/bt_factory.h"

namespace nb = nanobind;
using namespace nb::literals;

namespace pybt {

// --------------------------------------------------------------------------
// Live-tree tracking + atexit halt
// --------------------------------------------------------------------------

class LiveTreeRegistry
{
public:
  static LiveTreeRegistry& get()
  {
    static LiveTreeRegistry r;
    return r;
  }

  void add(std::weak_ptr<BT::Tree> tree)
  {
    std::lock_guard<std::mutex> lock(mu_);
    trees_.push_back(std::move(tree));
  }

  void halt_all()
  {
    std::lock_guard<std::mutex> lock(mu_);
    for(auto& w : trees_)
    {
      if(auto t = w.lock())
      {
        try
        {
          t->haltTree();
        }
        catch(...)
        {
          // Swallow during shutdown — we can't surface errors here.
        }
      }
    }
    trees_.clear();
  }

private:
  std::mutex mu_;
  std::vector<std::weak_ptr<BT::Tree>> trees_;
};

// Called from bind_factory.cpp.
void register_live_tree(std::shared_ptr<BT::Tree> tree)
{
  LiveTreeRegistry::get().add(tree);
}

namespace {

void on_python_exit()
{
  LiveTreeRegistry::get().halt_all();
}

#if defined(__unix__) || defined(__APPLE__)
// Captured at module init. If getpid() differs from this when a tick is
// attempted, we know we're running in a forked child — and BT.CPP holds
// state (parallel-node threads, atomic flags, signal handlers) that does
// not survive fork. Detect and refuse rather than crash unpredictably.
pid_t g_startup_pid = 0;

void detect_fork_or_throw()
{
  if(g_startup_pid != 0 && getpid() != g_startup_pid)
  {
    throw BT::RuntimeError("pybt is not fork-safe — create the tree in the "
                           "child process");
  }
}
#else
inline void detect_fork_or_throw() {}
#endif

// tick_while_running re-implemented to interleave PyErr_CheckSignals
// between ticks. Mirrors Tree::tickWhileRunning but with signal-check.
BT::NodeStatus
tick_while_running_with_signals(BT::Tree& self, std::chrono::milliseconds sleep_dur)
{
  detect_fork_or_throw();

  BT::NodeStatus status = BT::NodeStatus::IDLE;

  // Drop the GIL for the whole loop. Trampolines reacquire when they
  // need to call into Python. Signal checks reacquire briefly per iter.
  {
    nb::gil_scoped_release no_gil;

    do
    {
      status = self.tickOnce();

      if(status == BT::NodeStatus::RUNNING)
      {
        self.sleep(sleep_dur);
      }

      // Signal check — reacquire GIL just long enough.
      {
        nb::gil_scoped_acquire gil;
        if(PyErr_CheckSignals() != 0)
        {
          // GIL acquired and an exception is set (e.g. KeyboardInterrupt).
          // Halt the tree before throwing so background work stops cleanly.
          {
            nb::gil_scoped_release release_for_halt;
            self.haltTree();
          }
          throw nb::python_error();
        }
      }
    } while(status == BT::NodeStatus::RUNNING);
  }

  return status;
}

}  // namespace

void register_tree(nb::module_& m)
{
  // Register atexit halt — once per interpreter.
  static bool atexit_registered = false;
  if(!atexit_registered)
  {
    Py_AtExit(&on_python_exit);
    atexit_registered = true;
  }

#if defined(__unix__) || defined(__APPLE__)
  // Capture startup pid for fork-safety detection (see detect_fork_or_throw).
  if(g_startup_pid == 0)
  {
    g_startup_pid = getpid();
  }
#endif

  nb::class_<BT::Tree>(m, "Tree",
                       "An instantiated behavior tree. Construct via the "
                       "factory's `create_tree*` methods. Trees are owned by "
                       "Python; when garbage-collected, all nodes are halted "
                       "and destroyed.")

      .def("tick_once", &BT::Tree::tickOnce,
           "Tick the root once (or repeatedly within a single call if a node "
           "wakes the tree). Releases the GIL while ticking. Returns the "
           "resulting status.")

      .def("tick_exactly_once", &BT::Tree::tickExactlyOnce,
           "Tick the root exactly once, even if a node calls "
           "emitWakeUpSignal(). Returns the resulting status.")

      .def(
          "tick_while_running",
          [](BT::Tree& self, int sleep_ms) {
            return tick_while_running_with_signals(
                self, std::chrono::milliseconds(sleep_ms));
          },
          "sleep_ms"_a = 10,
          "Tick repeatedly until the tree returns SUCCESS or FAILURE. Sleeps "
          "`sleep_ms` between iterations. Releases the GIL between ticks and "
          "checks for KeyboardInterrupt every iteration.")

      .def("halt_tree", &BT::Tree::haltTree,
           "Halt every running node in the tree.")

      .def("root_blackboard", &BT::Tree::rootBlackboard,
           "Return the root Blackboard (opaque in Phase 1 — full binding lands "
           "in a later phase).")

      .def("sleep",
           [](BT::Tree& self, int ms) {
             return self.sleep(std::chrono::milliseconds(ms));
           },
           "duration_ms"_a,
           "Sleep, interruptible by a wake signal. Returns True if a wake "
           "signal arrived before the timeout.")

      .def_prop_ro(
          "root_node",
          [](BT::Tree& self) -> BT::TreeNode* { return self.rootNode(); },
          nb::rv_policy::reference_internal,
          "The root TreeNode of this tree (or None if empty).");
}

}  // namespace pybt
