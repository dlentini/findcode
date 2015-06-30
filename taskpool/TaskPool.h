/* TaskPool -- */

#pragma once

#include "WorkerThread.h"
#include "Semaphore.h"
#include <vector>
#include <atomic>

namespace parallel {

struct TaskPool {
  TaskPool();
  ~TaskPool();
  TaskPool(const TaskPool &) = delete;
  TaskPool &operator=(const TaskPool &) = delete;

  bool start(int thread_count = -1);
  bool stop();

  void wait_for_workers_to_be_ready();
  void wake_workers();

  unsigned int worker_count() const { return _worker_count; }
  unsigned int thread_count() const { return _thread_count; }

public:
  WorkerThread threads[MAX_THREADS];
  std::atomic<TaskCompletion *> main_completion;
  std::atomic<bool> shutting_down;

  semaphore sleep_notification;
  semaphore wake_up_call;

private:
  bool workers_idle;
  int _thread_count;
  int _worker_count;
};
}