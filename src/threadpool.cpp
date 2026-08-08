#include "threadpool.hpp"

ThreadPool::ThreadPool(size_t numThreads) {
    for (size_t i = 0; i < numThreads; ++i) {
        workers_.emplace_back([this] { workerLoop(); });
    }
}

void ThreadPool::workerLoop() {
    while (true) {
        std::function<void()> task;
        {
            std::unique_lock<std::mutex> lock(mutex_);
            cv_.wait(lock, [this] { return stop_ || !tasks_.empty(); });
            if (stop_ && tasks_.empty())
                return;                       // shutting down, nothing left → exit thread
            task = std::move(tasks_.front()); // take the work
            tasks_.pop();
        }                                     // ← lock released here
        task();                               // run OUTSIDE the lock
    }
}

void ThreadPool::enqueue(std::function<void()> task) {
    {
        std::lock_guard<std::mutex> lock(mutex_);
        tasks_.push(std::move(task));
    }
    cv_.notify_one();   // wake ONE worker
}

ThreadPool::~ThreadPool() {
    {
        std::lock_guard<std::mutex> lock(mutex_);
        stop_ = true;
    }
    cv_.notify_all();          // wake EVERY worker (they must all see stop_)
    for (std::thread& w : workers_)
        w.join();              // wait for each to finish and exit
}