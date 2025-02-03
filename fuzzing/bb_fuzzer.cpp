#include "behaviortree_cpp/blackboard.h"
#include <fuzzer/FuzzedDataProvider.h>
#include <iostream>
#include <vector>
#include <memory>
#include <thread>

#include <iomanip>

class ExceptionFilter
{
public:
  static bool isExpectedException(const std::exception& e)
  {
    const std::string what = e.what();
    const std::vector<std::string> expected_patterns = { "Blackboard::set",
                                                         "once declared, the type of a "
                                                         "port shall not change",
                                                         "Missing key",
                                                         "hasn't been initialized",
                                                         "Missing parent blackboard",
                                                         "Floating point truncated",
                                                         "Value outside the max "
                                                         "numerical limit",
                                                         "Value outside the lovest "
                                                         "numerical limit",
                                                         "Value is negative and can't be "
                                                         "converted to unsigned",
                                                         "Implicit casting to bool is "
                                                         "not allowed" };

    for(const auto& pattern : expected_patterns)
    {
      if(what.find(pattern) != std::string::npos)
      {
        return true;
      }
    }
    return false;
  }
};

class BlackboardFuzzer
{
private:
  std::vector<BT::Blackboard::Ptr> blackboards_;
  std::vector<std::string> generated_keys_;
  FuzzedDataProvider& fuzz_data_;

  std::string generateKey()
  {
    const std::string key_chars = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ01"
                                  "23456789_@";
    size_t length = fuzz_data_.ConsumeIntegralInRange<size_t>(1, 32);
    std::string key;
    for(size_t i = 0; i < length; ++i)
    {
      key +=
          key_chars[fuzz_data_.ConsumeIntegralInRange<size_t>(0, key_chars.length() - 1)];
    }
    generated_keys_.push_back(key);
    return key;
  }

  void fuzzSingleBB(BT::Blackboard::Ptr bb)
  {
    if(!bb)
      return;

    try
    {
      // Create random entry
      std::string key = generateKey();
      switch(fuzz_data_.ConsumeIntegralInRange<size_t>(0, 6))
      {
        case 0:
          bb->set(key, fuzz_data_.ConsumeIntegral<int>());
          break;
        case 1:
          bb->set(key, fuzz_data_.ConsumeFloatingPoint<double>());
          break;
        case 2:
          bb->set(key, fuzz_data_.ConsumeRandomLengthString());
          break;
        case 3:
          bb->set(key, fuzz_data_.ConsumeBool());
          break;
        case 4:
          bb->set(key, fuzz_data_.ConsumeIntegral<uint64_t>());
          break;
        case 5:
          bb->set(key, fuzz_data_.ConsumeFloatingPoint<float>());
          break;
        case 6: {
          // Try to get non-existent key
          bb->get<int>(generateKey());
          break;
        }
      }

      // Random operations on existing keys
      if(!generated_keys_.empty())
      {
        const auto& existing_key =
            generated_keys_[fuzz_data_.ConsumeIntegralInRange<size_t>(
                0, generated_keys_.size() - 1)];

        switch(fuzz_data_.ConsumeIntegralInRange<size_t>(0, 4))
        {
          case 0:
            bb->unset(existing_key);
            break;
          case 1:
            bb->getEntry(existing_key);
            break;
          case 2:
            bb->get<int>(existing_key);
            break;
          case 3:
            bb->get<double>(existing_key);
            break;
          case 4:
            bb->get<std::string>(existing_key);
            break;
        }
      }

      // Random remapping operations
      if(generated_keys_.size() >= 2)
      {
        size_t idx1 =
            fuzz_data_.ConsumeIntegralInRange<size_t>(0, generated_keys_.size() - 1);
        size_t idx2 =
            fuzz_data_.ConsumeIntegralInRange<size_t>(0, generated_keys_.size() - 1);
        bb->addSubtreeRemapping(generated_keys_[idx1], generated_keys_[idx2]);
      }
    }
    catch(const std::exception& e)
    {
      if(!ExceptionFilter::isExpectedException(e))
      {
        throw;
      }
    }
  }

  void createBlackboardHierarchy()
  {
    if(blackboards_.empty())
      return;

    auto parent = blackboards_[fuzz_data_.ConsumeIntegralInRange<size_t>(
        0, blackboards_.size() - 1)];

    auto child = BT::Blackboard::create(parent);
    if(fuzz_data_.ConsumeBool())
    {
      child->enableAutoRemapping(true);
    }

    blackboards_.push_back(child);
  }

  void fuzzJsonOperations(BT::Blackboard::Ptr bb)
  {
    try
    {
      auto json = BT::ExportBlackboardToJSON(*bb);
      if(fuzz_data_.ConsumeBool())
      {
        std::string json_str = json.dump();
        size_t pos = fuzz_data_.ConsumeIntegralInRange<size_t>(0, json_str.length());
        json_str.insert(pos, fuzz_data_.ConsumeRandomLengthString());
        json = nlohmann::json::parse(json_str);
      }
      BT::ImportBlackboardFromJSON(json, *bb);
    }
    catch(const std::exception& e)
    {
      if(!ExceptionFilter::isExpectedException(e))
      {
        throw;
      }
    }
  }

public:
  explicit BlackboardFuzzer(FuzzedDataProvider& provider) : fuzz_data_(provider)
  {
    blackboards_.push_back(BT::Blackboard::create());
  }

  void fuzz()
  {
    size_t num_operations = fuzz_data_.ConsumeIntegralInRange<size_t>(50, 200);

    for(size_t i = 0; i < num_operations && !blackboards_.empty(); ++i)
    {
      try
      {
        // Randomly select a blackboard to operate on
        size_t bb_idx =
            fuzz_data_.ConsumeIntegralInRange<size_t>(0, blackboards_.size() - 1);
        auto bb = blackboards_[bb_idx];

        switch(fuzz_data_.ConsumeIntegralInRange<size_t>(0, 3))
        {
          case 0:
            // Fuzz single blackboard operations
            fuzzSingleBB(bb);
            break;

          case 1:
            // Create new blackboards in hierarchy
            if(fuzz_data_.ConsumeBool())
            {
              createBlackboardHierarchy();
            }
            break;

          case 2:
            // JSON operations
            fuzzJsonOperations(bb);
            break;

          case 3:
            // Cleanup operations
            if(fuzz_data_.ConsumeBool() && blackboards_.size() > 1)
            {
              size_t remove_idx =
                  fuzz_data_.ConsumeIntegralInRange<size_t>(0, blackboards_.size() - 1);
              blackboards_.erase(blackboards_.begin() + remove_idx);
            }
            break;
        }
      }
      catch(const std::exception& e)
      {
        if(!ExceptionFilter::isExpectedException(e))
        {
          std::cerr << "Unexpected exception: " << e.what() << std::endl;
          throw;
        }
      }
    }
  }
};

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
  if(size < 64)
    return 0;

  try
  {
    FuzzedDataProvider fuzz_data(data, size);
    BlackboardFuzzer fuzzer(fuzz_data);
    fuzzer.fuzz();
  }
  catch(const std::exception& e)
  {
    if(!ExceptionFilter::isExpectedException(e))
    {
      std::cerr << "Unexpected top-level exception: " << e.what() << std::endl;
      return 1;
    }
  }

  return 0;
}
