#include "AsyncDispatcher.h"

AsyncDispatcher::~AsyncDispatcher() {
    //spdlog::info("AsyncDispatcher destroyed");
    delete impl;
}


int AsyncDispatcher::Subscribe(int target, SubscribeType  type, SubscribeCallback callback) {
    lock_guard<mutex>guadr(g_mtx);
    auto ret = subscribers.find(target);
    int cmd = 0;
    int sflags = 0;
    Subcriber* suber = nullptr;
    bool found = false;
    if (ret != subscribers.end()) {
        cmd = 1;
        sflags = ret->second->flags;
        suber = ret->second;
        found = true;
    }
    else {
        suber = new Subcriber(0);
    }

    int opflag;
    if (type == SubscribeType::Read) {
        opflag = EPOLLIN;
    }
    else if (type == SubscribeType::Write) {
        opflag = EPOLLOUT;
    }
    else {
        return -1;
    }

    if ((sflags & opflag) > 0) {
        //set before
        return 0;
    }
    sflags |= opflag;
    epoll_event ev;
    ev.events = sflags | DISPATCHER_DEFAULT_FLAGS;
    ev.data.ptr = static_cast<void*>(suber);

    int opret;
    //spdlog::info("target:{},epfd:{}", target, epfd);
    if (cmd == 1) {
        opret = impl->Modify(target, &ev);
    }
    else {
        opret = impl->Add(target, &ev);
    }

    if (opret < 0) {
        return -1;
    }

    if (!found) {
        subscribers[target] = suber;
    }
    subscribers[target]->flags = sflags;

    if (opflag & EPOLLIN) {
        subscribers[target]->callr = callback;
    }
    else {
        subscribers[target]->callw = callback;
    }
    return 0;
}

int AsyncDispatcher::Unsubscribe(int target, SubscribeType  type) {
    lock_guard<mutex>guadr(g_mtx);
    int opflag;
    if (type == SubscribeType::Read) {
        opflag = EPOLLIN;
    }
    else if (type == SubscribeType::Write) {
        opflag = EPOLLOUT;
    }
    auto ret = subscribers.find(target);
    bool find = ret != subscribers.end();
    int cmd;
    if (find) {
        if (ret->second->flags & opflag) {
            if ((ret->second->flags & ~opflag) == 0) {
                cmd = 1;
            }
            else {
                cmd = 0;
            }
            int exret;
            if (cmd == 1) {
                exret = impl->Delete(target, nullptr);
            }
            else {
                epoll_event evt;
                evt.events = ret->second->flags & ~opflag | DISPATCHER_DEFAULT_FLAGS;
                evt.data.ptr = ret->second;
                exret = impl->Modify(target, &evt);
            }
            if (exret < 0) {
                return -1;
            }
            if (cmd == 1) {
                subscribers.erase(target);
                delete ret->second;
            }
            else {
                ret->second->flags &= ~opflag;
            }
            return 0;
        }
        else {
            return 0;
        }
    }
    else {
        return 0;
    }
}

int AsyncDispatcher::Run() {
    impl->Wait();
    return 0;
}

int AsyncDispatcher::SubscribeAsyncRead(int target, SubscribeCallback callback) {
    return  Subscribe(target, SubscribeType::Read, callback);
}

int AsyncDispatcher::SubscribeAsyncWrite(int target, SubscribeCallback callback) {
    return  Subscribe(target, SubscribeType::Write, callback);
}

int AsyncDispatcher::UnsubscribeAsyncRead(int target) {
    return Unsubscribe(target, SubscribeType::Read);
}

int AsyncDispatcher::UnsubscribeAsyncWrite(int target) {
    return Unsubscribe(target, SubscribeType::Write);
}

int AsyncDispatcher::RemoveTargetSubscribe(int target) {
    std::unique_lock<mutex>lck(g_mtx);
    auto ret = subscribers.find(target);
    if (ret != subscribers.end()) {
        assert(impl->Delete(target, nullptr) == 0);
        Subcriber* suber = ret->second;
        suber->recycled = true;
        return 0;
    }
    return 0;
}

void AsyncDispatcher::RemoveTargetForever(int target) {
    std::unique_lock<mutex>lck(g_mtx);
    auto ret = subscribers.find(target);
    if (ret != subscribers.end()) {
        Subcriber* suber = ret->second;
        subscribers.erase(target);
        delete suber;
        //spdlog::error("recycle");
    }
}

AsyncDispatcher::DispatcherMetrics& AsyncDispatcher::GetMetrics() {
    std::unique_lock<mutex>lck(g_mtx);
    metrics.active_count = subscribers.size();
    return metrics;
}