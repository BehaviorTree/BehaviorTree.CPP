#pragma once

#include <array>
#include <future>
#include "behaviortree_cpp_v3/loggers/abstract_logger.h"
#include "behaviortree_cpp_v3/loggers/groot2_protocol.h"

namespace BT
{

/**
 * @brief The Groot2Publisher is used tt create an interface between
 * your BT.CPP executor and Groot2.
 *
 * An inter-process communication mechanism allow the two processes
 * to communicate throught a TCP port. The user should provide the
 * port to be used in the constructor.
 */
class Groot2Publisher : public StatusChangeLogger
{
  static std::mutex used_ports_mutex;
  static std::set<unsigned> used_ports;

  public:
  Groot2Publisher(const BT::Tree& tree, unsigned server_port = 1667);

  ~Groot2Publisher() override;

  Groot2Publisher(const Groot2Publisher& other) = delete;
  Groot2Publisher& operator=(const Groot2Publisher& other) = delete;

  Groot2Publisher(Groot2Publisher&& other) = default;
  Groot2Publisher& operator=(Groot2Publisher&& other)  = default;

  private:

  void callback(Duration timestamp,
                const TreeNode& node,
                NodeStatus prev_status,
                NodeStatus status) override;

  void flush() override;

  void serverLoop();

  void updateStatusBuffer();

  struct PImpl;
  std::unique_ptr<PImpl> _p;

};
}   // namespace BT

