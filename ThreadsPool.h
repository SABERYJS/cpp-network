#ifndef MT_THREADS_POLL_H
#define MT_THREADS_POLL_H

#include<functional>
#include<iostream>
#include<memory>
#include<atomic>
#include<thread>
#include<vector>
#include<condition_variable>
#include<queue>
#include<future>
#include<stdexcept>
#include<spdlog/spdlog.h>

using namespace std;


class ThreadsPool {
private:
    typedef std::function<void()> QueueType;
    vector<shared_ptr<thread>>handlers;
    atomic_bool stopped;
    int threads_count;
    queue<QueueType>tasks_;
    std::mutex g_mtx;
    std::mutex data_mtx;
    std::mutex thd_mtx;
    std::condition_variable cond;

public:
    ThreadsPool(int count) :threads_count(count), stopped(false) {
        if (threads_count <= 0) {
            throw std::runtime_error("init threads pool failed");
        }
        initHandlers();
    }
    ~ThreadsPool() {}
    ThreadsPool(const ThreadsPool&) = delete;
    ThreadsPool(ThreadsPool&&) = delete;
    ThreadsPool& operator=(const ThreadsPool&) = delete;
    ThreadsPool& operator=(ThreadsPool&&) = delete;

    bool CheckStopped() {
        return stopped;
    }

    template<class F, class ... Args> auto  Async(F&& f, Args&& ... args) -> std::future<decltype(f(args...))> {
        using return_t = decltype(f(args...));
        using future_t = std::future<return_t>;
        using task_t = std::packaged_task<return_t()>;

        std::lock_guard<mutex>lck(g_mtx);


        if (CheckStopped()) {
            throw std::runtime_error("threadspool stopped");
        }

        auto bind_func = std::bind(std::forward<F>(f), std::forward<Args>(args)...);
        std::shared_ptr<task_t> task = std::make_shared<task_t>(std::move(bind_func));
        future_t fut = task->get_future();
        {
            std::lock_guard<mutex>guard(data_mtx);
            tasks_.emplace([task]() -> void { (*task)(); });
        }
        cond.notify_one();
        return fut;
    }

    void Terminate() {
        g_mtx.lock();
        if (stopped) {
            g_mtx.unlock();
            return;
        }
        stopped = true;
        g_mtx.unlock();
        cond.notify_all();
        for (auto&& hld : handlers) {
            hld->join();
        }
    }
private:
    void initHandlers() {
        for (int i = 0;i < threads_count;i++) {
            handlers.push_back(std::shared_ptr<thread>(new thread(std::move(std::bind(&ThreadsPool::spawn, this)))));
        }
    }
    void spawn() {
        spdlog::info("create dispatch thread");
        while (true) {
            std::unique_lock<mutex>ulk(thd_mtx);
            bool find = false;
            QueueType task;
            cond.wait(ulk, [this, &find, &task]() {
                if (this->stopped) {
                    return true;
                }
                if (this->tasks_.empty()) {
                    return false;
                }
                {
                    std::lock_guard<mutex>guard(this->data_mtx);
                    task = std::move(this->tasks_.front());
                    this->tasks_.pop();
                    find = true;
                }
                return  true;
                });

            if (find) {
                task.operator()();
            }
            else {
                return;
            }
        }

    }
};


#endif