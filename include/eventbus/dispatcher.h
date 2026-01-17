/**
 * @file dispatcher.h
 * @brief Event dispatching implementations for different execution strategies
 *
 * This file provides various dispatcher implementations that control how
 * events are executed. Dispatchers handle the scheduling and execution
 * of event handler tasks, supporting different threading models and
 * execution strategies.
 *
 * Available dispatchers:
 * - SynchronousDispatcher: Executes tasks immediately in the calling thread
 * - SingleThreadDispatcher: Executes tasks in a dedicated background thread
 * - ThreadPoolDispatcher: Executes tasks using a thread pool
 * - ExternalDispatcher: Allows integration with external executor frameworks
 */

#pragma once

#include <atomic>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

namespace eventbus {

/**
 * @brief Abstract base class for event dispatchers
 *
 * Dispatcher defines the interface for scheduling and executing event handler tasks.
 * Different implementations provide various execution strategies (synchronous,
 * asynchronous, thread pool, etc.).
 */
class Dispatcher {
 public:
  /** Virtual destructor for proper cleanup of derived classes */
  virtual ~Dispatcher() = default;

  /**
   * @brief Dispatch a task for execution
   *
   * Schedules a task (typically an event handler) for execution. The execution
   * timing and threading depends on the concrete dispatcher implementation.
   *
   * @param task The task to execute (typically an event handler lambda)
   */
  virtual void dispatch(std::function<void()> task) = 0;

  /**
   * @brief Shutdown the dispatcher
   *
   * Initiates shutdown of the dispatcher, stopping acceptance of new tasks
   * and potentially waiting for existing tasks to complete.
   *
   * @note This method is noexcept and will not throw exceptions
   */
  virtual void shutdown() noexcept = 0;
};

/**
 * @brief Synchronous dispatcher that executes tasks immediately
 *
 * SynchronousDispatcher executes tasks immediately in the calling thread.
 * This provides blocking, sequential execution where publish() calls wait
 * for all event handlers to complete before returning.
 *
 * Use this dispatcher when:
 * - You need deterministic execution order
 * - Event processing should not be asynchronous
 * - Handler execution time is minimal
 * - You want simple, predictable behavior
 */
class SynchronousDispatcher : public Dispatcher {
 public:
  /** Default constructor */
  SynchronousDispatcher() = default;

  /** Default destructor */
  ~SynchronousDispatcher() override = default;

  /**
   * @brief Execute task immediately in current thread
   * @param task The task to execute immediately
   */
  void dispatch(std::function<void()> task) override {
    if (task && !shutdown_.load()) {
      task();  // Execute immediately in current thread
    }
  }

  /** Shutdown the dispatcher */
  void shutdown() noexcept override { shutdown_.store(true); }

 private:
  /** Shutdown flag */
  std::atomic<bool> shutdown_ = false;
};

/**
 * @brief Single-threaded asynchronous dispatcher
 *
 * SingleThreadDispatcher executes tasks in a dedicated background thread.
 * Tasks are queued and processed sequentially in the background, providing
 * non-blocking publish() operations while maintaining execution order.
 *
 * Use this dispatcher when:
 * - You want asynchronous processing but need sequential execution
 * - Event handlers may be slow and shouldn't block publishers
 * - You need guaranteed execution order for events
 * - Single-threaded execution is sufficient for your throughput needs
 */
class SingleThreadDispatcher : public Dispatcher {
 public:
  /**
   * @brief Constructor starting the background worker thread
   */
  SingleThreadDispatcher() : shutdown_(false) { worker_ = std::thread(&SingleThreadDispatcher::run, this); }

  /**
   * @brief Destructor that shuts down the worker thread
   */
  ~SingleThreadDispatcher() override {
    shutdown();
    if (worker_.joinable()) {
      worker_.join();
    }
  }

  /**
   * @brief Queue task for execution in background thread
   * @param task The task to execute asynchronously
   */
  void dispatch(std::function<void()> task) override {
    if (shutdown_.load()) return;

    {
      std::scoped_lock lock(mutex_);
      tasks_.push(std::move(task));
    }
    cv_.notify_one();
  }

  /** Shutdown the dispatcher and worker thread */
  void shutdown() noexcept override {
    shutdown_.store(true);
    cv_.notify_one();
  }

 private:
  void run() {
    while (!shutdown_.load()) {
      std::function<void()> task;
      {
        std::unique_lock lock(mutex_);
        cv_.wait(lock, [this] { return shutdown_.load() || !tasks_.empty(); });

        if (shutdown_.load() && tasks_.empty()) break;

        if (!tasks_.empty()) {
          task = std::move(tasks_.front());
          tasks_.pop();
        }
      }

      if (task) {
        task();
      }
    }
  }

  std::queue<std::function<void()>> tasks_;
  std::mutex mutex_;
  std::condition_variable cv_;
  std::thread worker_;
  std::atomic<bool> shutdown_ = false;
};

/**
 * @brief Thread pool dispatcher for high-throughput event processing
 *
 * ThreadPoolDispatcher uses a fixed-size thread pool to execute tasks concurrently.
 * This provides high throughput for applications with many concurrent events
 * and handlers that can run in parallel.
 *
 * @tparam ThreadCount Number of worker threads in the pool
 *
 * Use this dispatcher when:
 * - You need high throughput and can benefit from parallel execution
 * - Event handlers are CPU-intensive and can run concurrently
 * - Execution order is not critical
 * - You want to limit resource usage with a fixed thread count
 */
template <size_t ThreadCount>
class ThreadPoolDispatcher : public Dispatcher {
 public:
  /**
   * @brief Constructor creating a thread pool with specified number of threads
   */
  ThreadPoolDispatcher() : shutdown_(false) {
    for (size_t i = 0; i < ThreadCount; ++i) {
      workers_.emplace_back(&ThreadPoolDispatcher::worker_loop, this);
    }
  }

  /**
   * @brief Destructor that shuts down all worker threads
   */
  ~ThreadPoolDispatcher() override {
    shutdown();
    for (auto &worker : workers_) {
      if (worker.joinable()) {
        worker.join();
      }
    }
  }

  /**
   * @brief Queue task for execution in thread pool
   * @param task The task to execute in one of the worker threads
   */
  void dispatch(std::function<void()> task) override {
    if (shutdown_.load()) return;

    {
      std::scoped_lock lock(mutex_);
      tasks_.push(std::move(task));
    }
    cv_.notify_one();
  }

  /** Shutdown the dispatcher and all worker threads */
  void shutdown() noexcept override {
    shutdown_.store(true);
    cv_.notify_all();
  }

 private:
  void worker_loop() {
    while (!shutdown_.load()) {
      std::function<void()> task;
      {
        std::unique_lock lock(mutex_);
        cv_.wait(lock, [this] { return shutdown_.load() || !tasks_.empty(); });

        if (shutdown_.load() && tasks_.empty()) break;

        if (!tasks_.empty()) {
          task = std::move(tasks_.front());
          tasks_.pop();
          active_tasks_.fetch_add(1);
        }
      }

      if (task) {
        task();
        active_tasks_.fetch_sub(1);
      }
    }
  }

  std::queue<std::function<void()>> tasks_;
  std::mutex mutex_;
  std::condition_variable cv_;
  std::vector<std::thread> workers_;
  std::atomic<size_t> active_tasks_ = 0;
  std::atomic<bool> shutdown_ = false;
};

/**
 * @brief External dispatcher for integration with custom executor frameworks
 *
 * ExternalDispatcher allows integration with external executor frameworks
 * such as boost::asio, Qt event loops, or custom thread pools. You provide
 * a callback function that will be used to schedule tasks in your external
 * executor.
 *
 * Use this dispatcher when:
 * - You need to integrate with an existing executor framework
 * - You want custom scheduling logic (priorities, deadlines, etc.)
 * - You need integration with GUI frameworks or other event loops
 * - You want to reuse existing thread pool infrastructure
 */
class ExternalDispatcher : public Dispatcher {
 public:
  /**
   * @brief Constructor with external executor callback
   * @param executor Callback function that schedules tasks in external executor
   */
  explicit ExternalDispatcher(std::function<void(std::function<void()>)> executor)
      : executor_(std::move(executor)), shutdown_(false) {}

  /** Default destructor */
  ~ExternalDispatcher() override = default;

  /**
   * @brief Dispatch task using external executor
   * @param task The task to dispatch to the external executor
   */
  void dispatch(std::function<void()> task) override {
    if (shutdown_.load()) return;
    executor_(std::move(task));
  }

  /** Shutdown the dispatcher */
  void shutdown() noexcept override { shutdown_.store(true); }

 private:
  /** External executor callback function */
  std::function<void(std::function<void()>)> executor_;
  /** Shutdown flag */
  std::atomic<bool> shutdown_ = false;
};

template class ThreadPoolDispatcher<2>;
template class ThreadPoolDispatcher<4>;
template class ThreadPoolDispatcher<8>;

}  // namespace eventbus
