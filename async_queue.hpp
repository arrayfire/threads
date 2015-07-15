#include <thread>
#include <mutex>
#include <queue>
#include <atomic>
#include <functional>
#include <condition_variable>

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
    condition_variable cond;

    void queue_runner() {
        std::cout << "Starting: " << std::this_thread::get_id() << std::endl;
        while (!done) {
            unique_lock<mutex> lock(queue_mutex);
            cond.wait(lock, [this]() {
                return work_queue.empty() == false || done.load();
            });
            if(done) break;
            function<void()> func = work_queue.front();
            func();
            work_queue.pop();
        }
    }

public:
    /// \brief Enqueues a new function onto the work queue
    ///
    /// \param func A function which will be enqueued on the work queue
    /// \param args The argument of the funciton \p func
    template <typename F, typename... Args>
    void enqueue(F func, Args... args) {
        auto no_arg_func = std::bind(func, args...);
        {
            lock_guard<mutex> lock(queue_mutex);
            work_queue.push(no_arg_func);
        }
        cond.notify_one();
        return;
    }

    /// \brief Blocks until all work has completed
    ///
    /// This function will block the calling thread until all of the queued
    /// functions have completed
    void sync() {
        std::cout << "Syncing" << std::endl;
        while (work_queue.empty() == false)
            std::this_thread::yield();
        std::cout << "Done Syncing" << std::endl;
    }

    /// \brief Creates a new work queue
    async_queue() : done(false) {
        thread tmp(&async_queue::queue_runner, this);
        swap(queue_thread, tmp);
    }

    ~async_queue() {
        done.store(true);
        cond.notify_one();
        queue_thread.join();
    }
};
