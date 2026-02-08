/* Copyright (C) 2024 Davide Faconti - All Rights Reserved
 *
 * Tests for Blackboard thread-safety bugs.
 * These tests reproduce data races that are detectable by ThreadSanitizer (TSan).
 * Under normal (non-TSan) builds, they exercise the racy code paths but may
 * not always crash — TSan is required for reliable detection.
 */

#include "behaviortree_cpp/blackboard.h"

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <thread>

#include <gtest/gtest.h>

using namespace BT;

// BUG-2: Blackboard::set() existing-entry path takes a raw reference to
// the Entry, then unlocks storage_mutex_. If another thread calls unset()
// on the same key in that window, the shared_ptr in the map is erased
// and the Entry may be destroyed, leaving a dangling reference.
//
// This test hammers concurrent set() + unset() on the same key.
// Under TSan it will report a data race / use-after-free before the fix.
TEST(BlackboardThreadSafety, SetAndUnsetRace_Bug2)
{
  auto bb = Blackboard::create();

  // Pre-create the entry so set() takes the existing-entry branch
  bb->set("key", 0);

  std::atomic<bool> stop{ false };
  constexpr int kIterations = 5000;

  auto setter = [&]() {
    for(int i = 0; i < kIterations && !stop; i++)
    {
      // This creates the entry if it was unset, or updates it
      bb->set("key", i);
    }
  };

  auto unsetter = [&]() {
    for(int i = 0; i < kIterations && !stop; i++)
    {
      bb->unset("key");
    }
  };

  std::thread t1(setter);
  std::thread t2(unsetter);

  t1.join();
  t2.join();

  // If we get here without crashing/TSan error, the test passes
  SUCCEED();
}

// BUG-1 + BUG-8: Blackboard::set() new-entry path.
// After createEntryImpl() inserts the entry into storage_, the code writes
// entry->value, entry->sequence_id, entry->stamp WITHOUT holding entry_mutex.
// The entry is already visible to other threads via getEntry().
//
// The race is between set()'s unlocked write (lines 305-307) and a reader
// that obtained a shared_ptr<Entry> via getEntry() and reads entry members
// under entry_mutex. Since set() doesn't hold entry_mutex, TSan detects the race.
//
// Strategy: pre-create entries so getEntry() works, then concurrently
// unset+set (to force new-entry path) while reader holds the entry.
TEST(BlackboardThreadSafety, SetNewEntryWhileReading_Bug1_Bug8)
{
  constexpr int kIterations = 2000;

  auto bb = Blackboard::create();

  std::atomic<bool> stop{ false };

  // Writer: keeps cycling between unset and set to trigger the new-entry path
  auto writer = [&]() {
    for(int i = 0; i < kIterations && !stop; i++)
    {
      std::string key = "key_" + std::to_string(i % 5);
      bb->unset(key);
      bb->set(key, i);
    }
  };

  // Reader: grabs the entry via getEntry() and reads members under entry_mutex.
  // The race is that set() writes entry->value WITHOUT entry_mutex after
  // createEntryImpl() inserts the entry.
  auto reader = [&]() {
    for(int i = 0; i < kIterations && !stop; i++)
    {
      std::string key = "key_" + std::to_string(i % 5);
      auto entry = bb->getEntry(key);
      if(entry)
      {
        std::unique_lock lk(entry->entry_mutex);
        volatile bool empty = entry->value.empty();
        volatile auto seq = entry->sequence_id;
        (void)empty;
        (void)seq;
      }
    }
  };

  std::thread t1(writer);
  std::thread t2(reader);

  t1.join();
  t2.join();

  SUCCEED();
}

// BUG-8 specifically: Two threads calling set() for the same NEW key.
// Both see key as missing, both call createEntryImpl(), then both write
// entry->value without entry_mutex.
TEST(BlackboardThreadSafety, TwoThreadsSetSameNewKey_Bug8)
{
  constexpr int kRounds = 500;

  for(int round = 0; round < kRounds; round++)
  {
    auto bb = Blackboard::create();
    std::string key = "new_key";

    // Use a simple latch so both threads call set() at nearly the same time
    std::mutex mtx;
    std::condition_variable cv;
    int ready_count = 0;

    auto thread_func = [&](int value) {
      {
        std::unique_lock lk(mtx);
        ready_count++;
        cv.notify_all();
        cv.wait(lk, [&] { return ready_count >= 2; });
      }
      bb->set(key, value);
    };

    std::thread t1(thread_func, 1);
    std::thread t2(thread_func, 2);

    t1.join();
    t2.join();

    // The value should be one of the two written values
    int result = bb->get<int>(key);
    ASSERT_TRUE(result == 1 || result == 2);
  }
}

// BUG-3: cloneInto() reads/writes entry members (value, info, string_converter,
// sequence_id, stamp) while holding storage_mutex_ but NOT entry_mutex.
// A concurrent thread that already holds a shared_ptr<Entry> (obtained
// before cloneInto starts) can read entry members under entry_mutex,
// which doesn't synchronize with storage_mutex_.
//
// Strategy: grab entry shared_ptrs first, then start cloneInto in parallel
// with reads under entry_mutex.
TEST(BlackboardThreadSafety, CloneIntoWhileReading_Bug3)
{
  auto src = Blackboard::create();
  auto dst = Blackboard::create();

  constexpr int kEntries = 20;

  // Pre-populate both blackboards
  for(int i = 0; i < kEntries; i++)
  {
    std::string key = "key_" + std::to_string(i);
    src->set(key, i);
    dst->set(key, i * 10);
  }

  // Pre-grab entry shared_ptrs from dst so reader doesn't need storage_mutex_
  std::vector<std::shared_ptr<Blackboard::Entry>> dst_entries;
  for(int i = 0; i < kEntries; i++)
  {
    dst_entries.push_back(dst->getEntry("key_" + std::to_string(i)));
  }

  constexpr int kIterations = 2000;
  std::atomic<bool> stop{ false };

  auto cloner = [&]() {
    for(int i = 0; i < kIterations && !stop; i++)
    {
      src->cloneInto(*dst);
    }
  };

  // Reader holds pre-obtained entry shared_ptrs and reads under entry_mutex
  auto reader = [&]() {
    for(int i = 0; i < kIterations && !stop; i++)
    {
      auto& entry = dst_entries[i % kEntries];
      std::unique_lock lk(entry->entry_mutex);
      volatile bool empty = entry->value.empty();
      volatile auto seq = entry->sequence_id;
      (void)empty;
      (void)seq;
    }
  };

  std::thread t1(cloner);
  std::thread t2(reader);

  t1.join();
  t2.join();

  SUCCEED();
}

// BUG-4: ImportBlackboardFromJSON writes entry->value without entry_mutex.
// The reader holds a pre-obtained shared_ptr<Entry> and reads under entry_mutex,
// which doesn't synchronize with ImportBlackboardFromJSON's unprotected write.
TEST(BlackboardThreadSafety, ImportJsonWhileReading_Bug4)
{
  auto bb = Blackboard::create();

  // Pre-populate
  bb->set("int_val", 42);
  bb->set("str_val", std::string("hello"));

  // Export to JSON
  auto json = ExportBlackboardToJSON(*bb);

  // Pre-grab entry so reader doesn't need storage_mutex_
  auto entry = bb->getEntry("int_val");

  constexpr int kIterations = 2000;

  auto importer = [&]() {
    for(int i = 0; i < kIterations; i++)
    {
      ImportBlackboardFromJSON(json, *bb);
    }
  };

  // Reader holds pre-obtained entry and reads under entry_mutex
  auto reader = [&]() {
    for(int i = 0; i < kIterations; i++)
    {
      std::unique_lock lk(entry->entry_mutex);
      volatile bool empty = entry->value.empty();
      (void)empty;
    }
  };

  std::thread t1(importer);
  std::thread t2(reader);

  t1.join();
  t2.join();

  SUCCEED();
}

// BUG-5: debugMessage() iterates storage_ without holding storage_mutex_.
// Concurrent modification (insert/erase) causes iterator invalidation / UB.
TEST(BlackboardThreadSafety, DebugMessageWhileModifying_Bug5)
{
  auto bb = Blackboard::create();

  constexpr int kIterations = 500;

  auto modifier = [&]() {
    for(int i = 0; i < kIterations; i++)
    {
      std::string key = "key_" + std::to_string(i % 50);
      bb->set(key, i);
      if(i % 3 == 0)
      {
        bb->unset(key);
      }
    }
  };

  auto debugger = [&]() {
    for(int i = 0; i < kIterations; i++)
    {
      // Suppress stdout noise
      testing::internal::CaptureStdout();
      bb->debugMessage();
      testing::internal::GetCapturedStdout();
    }
  };

  std::thread t1(modifier);
  std::thread t2(debugger);

  t1.join();
  t2.join();

  SUCCEED();
}

// BUG-6: getKeys() iterates storage_ without holding storage_mutex_.
// Also returns StringView into map keys which can dangle if entries are erased.
TEST(BlackboardThreadSafety, GetKeysWhileModifying_Bug6)
{
  auto bb = Blackboard::create();

  constexpr int kIterations = 1000;

  auto modifier = [&]() {
    for(int i = 0; i < kIterations; i++)
    {
      std::string key = "key_" + std::to_string(i % 50);
      bb->set(key, i);
    }
  };

  auto key_reader = [&]() {
    for(int i = 0; i < kIterations; i++)
    {
      auto keys = bb->getKeys();
      // Just access the keys to detect any race
      volatile size_t count = keys.size();
      (void)count;
    }
  };

  std::thread t1(modifier);
  std::thread t2(key_reader);

  t1.join();
  t2.join();

  SUCCEED();
}
