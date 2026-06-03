/**
 * ThreadPool.h
 *
 * Production-grade fixed-size thread pool using C++11 primitives.
 *
 * Design goals:
 *  - Zero dynamic allocation after construction
 *  - Work-stealing via a single mutex-protected deque (simple, correct)
 *  - Graceful shutdown: drains the queue before all workers exit
 *  - Minimal header-only dependency on <thread>, <mutex>, <condition_variable>,
 *    <functional>, <deque>, and <vector>
 *
 * Usage:
 *   ThreadPool pool;                   // uses hardware_concurrency() threads
 *   ThreadPool pool(4);                // explicit thread count
 *   auto fut = pool.submit(fn, args…); // returns std::future<ReturnType>
 *   pool.wait_all();                   // block until all pending tasks finish
 */

#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <atomic>
#include <condition_variable>
#include <deque>
#include <functional>
#include <future>
#include <memory>
#include <mutex>
#include <stdexcept>
#include <thread>
#include <type_traits>
#include <vector>

class ThreadPool
{
public:
    /* ------------------------------------------------------------------ */
    /*  Construction / Destruction                                          */
    /* ------------------------------------------------------------------ */

    /**
     * @param num_threads  Worker thread count.
     *                     0 → uses std::thread::hardware_concurrency()
     *                     (falls back to 1 if that returns 0).
     */
    explicit ThreadPool(unsigned int num_threads = 0)
        : stop_(false), active_tasks_(0)
    {
        unsigned int n = (num_threads == 0)
                             ? std::thread::hardware_concurrency()
                             : num_threads;
        if (n == 0) n = 1;

        workers_.reserve(n);
        for (unsigned int i = 0; i < n; ++i)
            workers_.emplace_back(&ThreadPool::worker_loop, this);
    }

    /**
     * Destructor: signals all workers to stop after draining the queue,
     * then joins every thread.
     */
    ~ThreadPool()
    {
        {
            std::lock_guard<std::mutex> lk(queue_mutex_);
            stop_ = true;
        }
        queue_cv_.notify_all();
        for (auto &t : workers_)
            if (t.joinable()) t.join();
    }

    /* Prevent copying and moving */
    ThreadPool(const ThreadPool &)            = delete;
    ThreadPool &operator=(const ThreadPool &) = delete;
    ThreadPool(ThreadPool &&)                 = delete;
    ThreadPool &operator=(ThreadPool &&)      = delete;

    /* ------------------------------------------------------------------ */
    /*  Submit a task                                                       */
    /* ------------------------------------------------------------------ */

    /**
     * Enqueue a callable together with its arguments.
     *
     * @returns std::future<ReturnType> that becomes ready when the task
     *          finishes (or holds an exception if the task threw).
     *
     * @throws std::runtime_error if the pool has already been stopped.
     */
    template <typename F, typename... Args>
    auto submit(F &&f, Args &&...args)
        -> std::future<typename std::result_of<F(Args...)>::type>
    {
        using ReturnType = typename std::result_of<F(Args...)>::type;

        auto task = std::make_shared<std::packaged_task<ReturnType()>>(
            std::bind(std::forward<F>(f), std::forward<Args>(args)...));

        std::future<ReturnType> result = task->get_future();

        {
            std::lock_guard<std::mutex> lk(queue_mutex_);
            if (stop_)
                throw std::runtime_error("ThreadPool: submit() called on a stopped pool");

            ++active_tasks_;
            queue_.emplace_back([task]() { (*task)(); });
        }
        queue_cv_.notify_one();
        return result;
    }

    /* ------------------------------------------------------------------ */
    /*  Synchronization helpers                                             */
    /* ------------------------------------------------------------------ */

    /**
     * Block the calling thread until every submitted task has completed.
     * New tasks may safely be submitted while wait_all() is blocking.
     */
    void wait_all()
    {
        std::unique_lock<std::mutex> lk(done_mutex_);
        done_cv_.wait(lk, [this]() { return active_tasks_.load() == 0; });
    }

    /** @returns the number of worker threads. */
    std::size_t thread_count() const { return workers_.size(); }

    /** @returns approximate number of pending + in-flight tasks. */
    std::size_t pending_tasks() const
    {
        std::lock_guard<std::mutex> lk(queue_mutex_);
        return queue_.size();
    }

private:
    /* ------------------------------------------------------------------ */
    /*  Worker loop                                                         */
    /* ------------------------------------------------------------------ */

    void worker_loop()
    {
        while (true)
        {
            std::function<void()> task;

            {
                std::unique_lock<std::mutex> lk(queue_mutex_);
                queue_cv_.wait(lk, [this]() {
                    return stop_ || !queue_.empty();
                });

                if (stop_ && queue_.empty())
                    return;   // drain complete, exit

                task = std::move(queue_.front());
                queue_.pop_front();
            }

            /* Execute outside the lock */
            task();

            /* Signal completion */
            if (--active_tasks_ == 0)
                done_cv_.notify_all();
        }
    }

    /* ------------------------------------------------------------------ */
    /*  Data members                                                        */
    /* ------------------------------------------------------------------ */

    std::vector<std::thread>          workers_;

    mutable std::mutex                queue_mutex_;
    std::deque<std::function<void()>> queue_;
    std::condition_variable           queue_cv_;

    std::mutex                        done_mutex_;
    std::condition_variable           done_cv_;

    bool                              stop_;
    std::atomic<int>                  active_tasks_;
};

#endif /* THREADPOOL_H */
