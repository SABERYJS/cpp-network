#ifndef MT_ASYNC_TIMER_H
#define MT_ASYNC_TIMER_H


#pragma once

#include<signal.h>
#include<time.h>
#include<functional>
#include<atomic>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include<stdexcept>
#include<iostream>
#include<spdlog/spdlog.h>
#include<mutex>



using namespace std;

class AsyncTimer
{
private:

    typedef std::function<void()> TaskType;
    TaskType task;
    timer_t timerid{ nullptr };
    sigevent sev{ 0 };
    std::atomic_bool canceled{false};
    sigset_t mask;
    struct sigaction sa;
    itimerspec spec;
    int interval;
    std::mutex mtx;

public:
    template<class F, class ...Args>
    AsyncTimer(int intval, F&& f, Args&&...args) :interval(intval), task(std::move(std::bind(std::forward<F>(f), std::forward<Args>(args)...))) {
        createTimer();
    }

    AsyncTimer(const AsyncTimer&) = delete;
    AsyncTimer(AsyncTimer&&) = delete;
    AsyncTimer& operator=(const AsyncTimer&) = delete;
    AsyncTimer& operator=(AsyncTimer&&) = delete;

    ~AsyncTimer() {}
private:
    void createTimer() {
        sev.sigev_notify = SIGEV_SIGNAL;
        sev.sigev_signo = SIGUSR1;
        sev.sigev_value.sival_ptr = static_cast<void*>(this);

        sa.sa_flags = SA_SIGINFO;
        sa.sa_sigaction = &AsyncTimer::handler;

        sigemptyset(&sa.sa_mask);
        if (sigaction(SIGUSR1, &sa, nullptr) != 0) {
            throw std::runtime_error("sigaction failed");
        }

        if (timer_create(CLOCK_MONOTONIC, &sev, &timerid) != 0) {
            throw std::runtime_error("timer_create failed");
        }

        spec.it_interval.tv_sec = interval;
        spec.it_interval.tv_nsec = 0;
        spec.it_value.tv_nsec = 0;
        spec.it_value.tv_sec = interval;

        if (timer_settime(timerid, 0, &spec, nullptr) != 0) {
            throw std::runtime_error("timer_settime failed");
        }
        //spdlog::info("timer started");
    }

    TaskType& getTask() {
        return task;
    }


    static  void handler(int sig, siginfo_t* si, void* uc) {
        AsyncTimer* timer = static_cast<AsyncTimer*>(si->si_value.sival_ptr);
        assert(timer != nullptr);
        timer->getTask()();
    }

public:
    void Stop() {
        std::unique_lock<mutex>lock(mtx);
        if (!canceled) {
            canceled = true;
            timer_delete(timerid);
        }
    }
};


#endif