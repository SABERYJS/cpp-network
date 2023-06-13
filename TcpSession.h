#ifndef MT_TCP_SESSION_H
#define MT_TCP_SESSION_H

#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include<iostream>
#include<string>
#include<memory>
#include<spdlog/spdlog.h>
#include<cerrno>
#include<cstring>
#include"AsyncDispatcher.h"
#include"DynamicBytes.h"
#include<functional>
#include<mutex>
#include<functional>


using namespace std;

class TcpSession :public std::enable_shared_from_this<TcpSession> {
private:
    sockaddr_in saddr;
    int sockfd{ 0 };
    AsyncDispatcher* dispatcher;
    DynamicBytes db_read;
    DynamicBytes db_write;
    bool closed{ false };
    mutex sync_mtx;

public:
    TcpSession(/* args */);
    ~TcpSession();
    int Socket()const {
        return sockfd;
    }
    sockaddr_in* Saddr() {
        return &saddr;
    }
    void Run(int sockfd, AsyncDispatcher* dispatcher);
    void WriteData(const char* ptr, size_t len);
    void WriteData(string&& data);
private:
    void readAsyncCallback(int error);
    void writeAsyncCallback(int error);

protected:
    void handleError(int error);
    virtual void ProcessRead(const char* ptr, int size) = 0;
    void setConsumed(int size);
    void setCommitted(int size);
    void startRead();
    void startWrite();
    void stopWrite();
    virtual void doInit() {}
    inline int get_fd()const { return sockfd; }
    inline bool check_closed() { return closed; }
    virtual void notifyWriteCall() {}
    //no mutx lock
    //so only called by read callback
    DynamicBytes& RefWriteDb(std::function<void(DynamicBytes& db)>ops);
    //call from other threads,do not call it from read callback
    //it will block cause deadlock
    DynamicBytes& RefWriteDbMutex(std::function<void(DynamicBytes& db)>ops);
    virtual void process_error(int error) {}
};

#endif