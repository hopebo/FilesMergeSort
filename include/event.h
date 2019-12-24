#ifndef EVENT_H_
#define EVENT_H_

#include <thread>
#include <mutex>
#include <condition_variable>

class Event
{
 public:
  Event() { }
  ~Event() { }
  void Wait()
  {
    std::unique_lock<std::mutex> lock(mutex_);
    cv_.wait(lock);
  }

  void Notify()
  {
    cv_.notify_one();
  }

  void Lock() {
    mutex_.lock();
  }

  void Unlock() {
    mutex_.unlock();
  }

 public:
  std::mutex mutex_;
  std::condition_variable cv_;
};

#endif  // EVENT_H_
