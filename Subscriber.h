
#ifndef MT_SUBSCRIBER_H
#define MT_SUBSCRIBER_H

#include<functional>
#include<memory>


typedef std::function<void(int)>SubscribeCallback;

struct Subcriber {
    int flags;
    SubscribeCallback callr;
    SubscribeCallback callw;
    Subcriber(int flags) :flags(flags) {}
    bool recycled{ false };
    std::mutex g_mtx;
    bool cleared{ false };

    std::unique_lock<std::mutex>auto_ptr() {
        return std::unique_lock<std::mutex>(g_mtx);
    }
    void do_clear() {
        if (recycled && !cleared) {
            //deference
            cleared = true;
            callr = callw = nullptr;
            spdlog::error("do_clear");
        }
    }
    inline bool pass() { return !recycled; }
};

enum SubscribeType {
    Read, Write
};


#endif
