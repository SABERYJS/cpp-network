
#include "SafeMessageQueue.h"
#include<memory>
#include<thread>
#include<chrono>
#include<iostream>

using namespace std;

class Message {
public:
    int number{0};
public:
    Message(int num) :number(num) {}
    ~Message() {}
};


int main(int argc, char const* argv[]) {
    SafeMessageQueue<std::shared_ptr<Message>>  smq;
    for (int i = 0;i < 2;i++) {
        thread t([&smq]() {
            for (int i = 0;i < 100000;i++) {
                smq.Publish(std::make_shared<Message>(i));
            }
            });
        t.detach();
    }
    for (int i = 0;i < 3;i++) {
        thread t([&smq]() {
            while (true) {
                std::shared_ptr<Message>msg = smq.Subscribe();
                cout << msg->number << endl;
            }
            });
        t.detach();
    }
    std::this_thread::sleep_for(std::chrono::seconds(100));
    return 0;
}
