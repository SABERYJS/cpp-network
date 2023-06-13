#ifndef MT_SAFE_MESSAGE_QUEUE_H
#define MT_SAFE_MESSAGE_QUEUE_H

#include<mutex>
#include<condition_variable>
#include<spdlog/spdlog.h>
#include<fmt/format.h>
#include<list>
#include<atomic>

using namespace std;

//support multiple thread,it is safe
template<class QueueType>
class SafeMessageQueue {
private:
    std::condition_variable cond;
    std::mutex data_mtx;
    std::mutex op_mtx;
    std::list<QueueType>data;
    std::atomic_bool stop{false};
    std::condition_variable stop_cond;
    std::mutex stop_mtx;
public:
    SafeMessageQueue(/* args */) {}
    ~SafeMessageQueue() {}
    void Publish(QueueType qt) {
        std::unique_lock<std::mutex>lock(data_mtx);
        data.push_back(qt);
        cond.notify_one();
    }
    QueueType Subscribe() {
        while (true) {
            data_mtx.lock();
            if (data.size() > 0) {
                QueueType qt = data.front();
                data.pop_front();
                data_mtx.unlock();
                return qt;
            }
            else {
                data_mtx.unlock();
                std::unique_lock<std::mutex>lock(op_mtx);
                QueueType qt = nullptr;
                cond.wait(lock, [this, &qt]() {
                    std::unique_lock<std::mutex>lock(data_mtx);
                    if (data.size() > 0) {
                        qt = data.front();
                        data.pop_front();
                        return true;
                    }
                    if (stop) {
                        return true;
                    }
                    return false;
                    });
                if (stop) {
                    stop_cond.notify_all();
                    return nullptr;
                }
                return qt;
            }
        }
    }
    void Size() {
        std::unique_lock<std::mutex>lock(data_mtx);
        return data.size();
    }

    void WaitStop() {
        std::unique_lock<std::mutex>lock(stop_mtx);
        if (stop) {
            return;
        }
        stop = true;
        stop_cond.wait(lock);
    }
};


#endif