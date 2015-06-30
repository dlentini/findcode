/*	SpinMutex -- Implementation of a spinning mutex using atomic operations.
 */

#pragma once

#include <atomic>
#include <mutex>

namespace parallel {

struct spin_mutex {
public:
  friend struct lock;

  spin_mutex() {}

  struct lock {
    lock(spin_mutex &mut) : mutex(mut) { mutex.aquire(); }

    ~lock() { mutex.release(); }

  private:
    spin_mutex &mutex;
  };

private:
  void aquire() {
    // while (locked.test_and_set(std::memory_order_acquire));
    mutex.lock();
  }

  void release() {
    // locked.clear(std::memory_order_release);
    mutex.unlock();
  }

private:
  // std::atomic_flag locked;
  std::mutex mutex;
};
}
