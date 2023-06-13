#ifndef MT_ASYNC_DISPATCHER_H
#define MT_ASYNC_DISPATCHER_H

#include <stdio.h>     
#include <unistd.h>   
#include <sys/epoll.h>
#include<map>
#include<mutex>
#include <functional>
#include<spdlog/spdlog.h>
#include<stdexcept>
#include <fmt/format.h>
#include"Subscriber.h"
#include "DispatcherImpl.h"
#include<assert.h>
#include "ThreadsPool.h"
#include<atomic>



using namespace std;

//default level trigger mode
#ifndef DISPATCHER_DEFAULT_FLAGS
#define DISPATCHER_DEFAULT_FLAGS  EPOLLRDHUP | EPOLLET
#endif


//reference https://suchprogramming.com/epoll-in-3-easy-steps/
//dispatch async events
//https://linuxhint.com/epoll-7-c-function/
//https://man7.org/linux/man-pages/man7/epoll.7.html
//https://man7.org/linux/man-pages/man2/epoll_wait.2.html
/*
EPOLLERR
              Error condition happened on the associated file
              descriptor.  This event is also reported for the write end
              of a pipe when the read end has been closed.

              epoll_wait(2) will always report for this event; it is not
              necessary to set it in events when calling epoll_ctl().
*/
class AsyncDispatcher {
public:
    struct DispatcherMetrics {
        int active_count{ 0 };
    };

private:
    map<int, Subcriber*>subscribers;
    DispatcherImpl* impl;
    mutex g_mtx;
    DispatcherMetrics metrics;
public:
    AsyncDispatcher(ThreadsPool* pool) {
        impl = new DispatcherImpl(pool);
    }
    ~AsyncDispatcher();
    int Subscribe(int target, SubscribeType  type, SubscribeCallback callback);
    int Unsubscribe(int target, SubscribeType  type);
    int SubscribeAsyncRead(int target, SubscribeCallback callback);
    int SubscribeAsyncWrite(int target, SubscribeCallback callback);
    int UnsubscribeAsyncRead(int target);
    int UnsubscribeAsyncWrite(int target);
    int RemoveTargetSubscribe(int target);
    void RemoveTargetForever(int target);
    int Run();
    AsyncDispatcher::DispatcherMetrics& GetMetrics();
};

#endif