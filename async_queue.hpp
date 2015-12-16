#pragma once

#include <thread>
#include <mutex>
#include <queue>
#include <atomic>
#include <functional>
#include <condition_variable>
#include <future>

#include <iostream>

using std::thread;
using std::atomic;
using std::mutex;
using std::condition_variable;
using std::unique_lock;
using std::lock_guard;

using std::queue;
using std::function;
using std::bind;

class async_queue {
    thread queue_thread;
    queue<function<void()>> work_queue;
    atomic<bool> done;
    mutex queue_mutex;
    mutex work_mutex;
    condition_variable cond;

    void queue_runner() {
        //std::cout << "Starting: " << std::this_thread::get_id() << std::endl;
        while (!done) {
            unique_lock<mutex> work_lock(work_mutex, std::defer_lock);
            function<void()> func;
            {
                unique_lock<mutex> lock(queue_mutex);
                cond.wait(lock, [this]() {
                        return work_queue.empty() == false || done;
                    });

                if(done) break;

                work_lock.lock();
                swap(func, work_queue.front());
                work_queue.pop();
            }
            if (!func) {printf("bad function: %zu\n", work_queue.size());}
            else func();
        }
    }

public:
    /// \brief Enqueues a new function onto the work queue
    ///
    /// \param func A function which will be enqueued on the work queue
    /// \param args The argument of the funciton \p func
    template <typename F, typename... Args>
    void enqueue(const F func, Args... args) {
        if(std::this_thread::get_id() == queue_thread.get_id()) {
            func(args...);
        } else {
            auto no_arg_func = std::bind(func, std::forward<Args>(args)...);
            {
                lock_guard<mutex> lock(queue_mutex);
                work_queue.push(no_arg_func);
            }

            cond.notify_one();
        }
        return;
    }

    /// \brief Check if the current thread of execution is same as the queue thread
    ///
    /// \return A boolean indicating if current thread is same as the queue thread
    bool is_worker() const {
        return std::this_thread::get_id() == queue_thread.get_id();
    }

    /// \brief Blocks until all work has completed
    ///
    /// This function will block the calling thread until all of the queued
    /// functions have completed
    void sync() {
        //std::cout << "Syncing" << std::endl;
        std::promise<void> p;
        std::future<void> fut = p.get_future();
        auto f = [] (std::promise<void>& pr) {
            pr.set_value();
        };
        this->enqueue(f, ref(p));
        fut.wait();
        //std::cout << "Done Syncing" << std::endl;
    }

    /// \brief Creates a new work queue
    async_queue()
        : work_queue()
        , done(false) {
        thread tmp(&async_queue::queue_runner, this);
        swap(queue_thread, tmp);
    }

    ~async_queue() {
        done.store(true);
        cond.notify_one();
        queue_thread.join();
    }
};
