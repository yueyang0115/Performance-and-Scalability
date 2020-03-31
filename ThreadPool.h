#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include <condition_variable>
#include <functional>
#include <future>
#include <memory>
#include <mutex>
#include <queue>
#include <stdexcept>
#include <thread>
#include <vector>

#include "function.h"
#include "thread_arg.h"

class ThreadPool {
 public:
  ThreadPool() : done(false) {
    auto numberOfThreads = std::thread::hardware_concurrency();
    if (numberOfThreads == 0) {
      numberOfThreads = 1;
    }

    for (unsigned i = 0; i < numberOfThreads; ++i) {
      threads.push_back(std::thread(&ThreadPool::doWork, this));
    }
  }

  ~ThreadPool() {
    done = true;

    workQueueConditionVariable.notify_all();
    for (auto & thread : threads) {
      if (thread.joinable()) {
        thread.join();
      }
    }
  }

  void queueWork(Thread_arg * thr_arg) {
    std::lock_guard<std::mutex> g(workQueueMutex);
    workQueue.push(thr_arg);
    workQueueConditionVariable.notify_one();
  }

 private:
  std::condition_variable_any workQueueConditionVariable;
  std::vector<std::thread> threads;
  std::mutex workQueueMutex;
  std::queue<Thread_arg *> workQueue;
  bool done;

  void doWork() {
    while (!done) {
      Thread_arg * thr_arg;
      {
        std::unique_lock<std::mutex> g(workQueueMutex);
        workQueueConditionVariable.wait(g, [&] { return !workQueue.empty() || done; });

        thr_arg = workQueue.front();
        workQueue.pop();
      }

      processRequest(thr_arg);
    }
  }
};
#endif
