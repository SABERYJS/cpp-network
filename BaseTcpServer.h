#ifndef BASE_SERVER_H
#define BASE_SERVER_H
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include<iostream>
#include<string>
#include "TcpSession.h"
#include<memory>
#include<spdlog/spdlog.h>
#include<cerrno>
#include<cstring>
#include <arpa/inet.h>
#include "CommonUtils.h"
#include<functional>
#include"AsyncDispatcher.h"
#include<stdexcept>
#include<mutex>


using namespace std;

#ifndef MT_TCP_SERVER_BACKLOG 
#define MT_TCP_SERVER_BACKLOG 256
#endif


class BaseTcpServer {
private:
    string host;
    short port;
    int sockfd;
    sockaddr_in saddr;
    AsyncDispatcher* dispatcher;
    mutex mtx;
public:
    BaseTcpServer(string host, short port, AsyncDispatcher* dispatcher) :host(host), port(port), dispatcher(dispatcher) {}
    ~BaseTcpServer();
    int Socket();
    int Bind();
    int Listen();
    void Accept(int error);

    void Run();
    int SetSocketReUseAddr();
    int SetSocketReUsePort();


protected:
    int SetOptions();
    virtual TcpSession* createSession() = 0;
};

#endif