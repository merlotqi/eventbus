#pragma once

#include <atomic>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <queue>
#include <thread>

namespace eventbus {

class Dispatcher {
 public:
  virtual ~Dispatcher() = default;
  virtual void dispatch(std::function<void()> task) = 0;
  virtual void shutdown() noexcept = 0;
};

class SynchronousDispatcher : public Dispatcher {
 public:
  SynchronousDispatcher() = default;
  ~SynchronousDispatcher() override = default;

  void dispatch(std::function<void()> task) override {
    if (task && !shutdown_.load()) {
      task();  // Execute immediately in current thread
    }
  }

  void shutdown() noexcept override { shutdown_.store(true); }

 private:
  std::atomic<bool> shutdown_ = false;
};

class SingleThreadDispatcher : public Dispatcher {
 public:
  SingleThreadDispatcher() : shutdown_(false) { worker_ = std::thread(&SingleThreadDispatcher::run, this); }

  ~SingleThreadDispatcher() override {
    shutdown();
    if (worker_.joinable()) {
      worker_.join();
    }
  }

  void dispatch(std::function<void()> task) override {
    if (shutdown_.load()) return;

    {
      std::scoped_lock lock(mutex_);
      tasks_.push(std::move(task));
    }
    cv_.notify_one();
  }

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

template <size_t ThreadCount>
class ThreadPoolDispatcher : public Dispatcher {
 public:
  ThreadPoolDispatcher() : shutdown_(false) {
    for (size_t i = 0; i < ThreadCount; ++i) {
      workers_.emplace_back(&ThreadPoolDispatcher::worker_loop, this);
    }
  }

  ~ThreadPoolDispatcher() override {
    shutdown();
    for (auto &worker : workers_) {
      if (worker.joinable()) {
        worker.join();
      }
    }
  }

  void dispatch(std::function<void()> task) override {
    if (shutdown_.load()) return;

    {
      std::scoped_lock lock(mutex_);
      tasks_.push(std::move(task));
    }
    cv_.notify_one();
  }

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

class ExternalDispatcher : public Dispatcher {
 public:
  explicit ExternalDispatcher(std::function<void(std::function<void()>)> executor)
      : executor_(std::move(executor)), shutdown_(false) {}

  ~ExternalDispatcher() override = default;

  void dispatch(std::function<void()> task) override {
    if (shutdown_.load()) return;
    executor_(std::move(task));
  }

  void shutdown() noexcept override { shutdown_.store(true); }

 private:
  std::function<void(std::function<void()>)> executor_;
  std::atomic<bool> shutdown_ = false;
};

template class ThreadPoolDispatcher<2>;
template class ThreadPoolDispatcher<4>;
template class ThreadPoolDispatcher<8>;

}  // namespace eventbus
