#ifndef MT_DISPATCHER_H
#define MT_DISPATCHER_H

#include <stdio.h>     
#include <unistd.h>   
#include <sys/epoll.h>
#include<map>
#include<mutex>
#include<functional>
#include<spdlog/spdlog.h>
#include<stdexcept>
#include <fmt/format.h>
#include"Subscriber.h"
#include<memory>
#include<cassert>
#include "ThreadsPool.h"

#ifndef DISPATCHER_WAIT_COUNT
#define DISPATCHER_WAIT_COUNT 65536
#endif

class DispatcherImpl {
private:
    int cfd;
    ThreadsPool* tPool;
public:
    DispatcherImpl(ThreadsPool* pool) :tPool(pool) {
        Create();
    }
    ~DispatcherImpl();
    int Create();
    int Add(int fd, epoll_event* evt);
    int Modify(int fd, epoll_event* evt);
    int Delete(int fd, epoll_event* evt);
    void Wait();
};

#endif