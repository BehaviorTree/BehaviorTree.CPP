#pragma once

#include <functional>
#include <memory>
#include <mutex>
#include <vector>

namespace BT
{
/** A signal supporting concurrent notification and subscription.
 * Callbacks run outside the subscription lock and may be invoked concurrently.
 * Dropping a subscriber prevents future calls, but does not wait for callbacks
 * already selected for invocation. The owner must synchronize their lifetime
 * with callback targets.
 */
template <typename... CallableArgs>
class ThreadSafeSignal
{
public:
  using CallableFunction = std::function<void(CallableArgs...)>;

  class Subscription
  {
    friend class ThreadSafeSignal;

  public:
    explicit Subscription(CallableFunction func)
      : callback_(std::make_shared<CallableFunction>(std::move(func)))
    {}

  private:
    std::shared_ptr<CallableFunction> callback_;
  };

  using Subscriber = std::shared_ptr<Subscription>;

  [[nodiscard]] Subscriber subscribe(CallableFunction func)
  {
    auto subscriber = std::make_shared<Subscription>(std::move(func));
    const std::scoped_lock lock(mutex_);
    removeExpired();
    subscribers_.emplace_back(subscriber);
    return subscriber;
  }

  void notify(CallableArgs... args)
  {
    std::vector<std::weak_ptr<Subscription>> subscribers;
    {
      const std::scoped_lock lock(mutex_);
      removeExpired();
      subscribers = subscribers_;
    }
    for(const auto& weak_subscriber : subscribers)
    {
      if(auto subscriber = weak_subscriber.lock())
      {
        // Keep the callable alive without keeping its subscription alive.
        // A callback can reset its own handle and recursively notify safely.
        auto callback = subscriber->callback_;
        subscriber.reset();
        (*callback)(args...);
      }
    }
  }

private:
  void removeExpired()
  {
    for(auto it = subscribers_.begin(); it != subscribers_.end();)
    {
      if(it->expired())
      {
        it = subscribers_.erase(it);
      }
      else
      {
        ++it;
      }
    }
  }

  std::mutex mutex_;
  std::vector<std::weak_ptr<Subscription>> subscribers_;
};
}  // namespace BT
