#ifndef MT_BASE_TCP_CLIENT_H
#define MT_BASE_TCP_CLIENT_H

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
#include <arpa/inet.h>
#include "CommonUtils.h"
#include<functional>
#include"AsyncDispatcher.h"
#include<stdexcept>


using namespace std;

class BaseTcpClient {
private:
    string server_addr;
    short server_port;
    int sockfd;
    sockaddr_in saddr;

    void createSocket();

    int write_base(const char* data, int len);
    int read_base(char* dest, int len);
public:
    BaseTcpClient(string saddr, short sport);
    ~BaseTcpClient();

    void Connect();

    int Send(const char* data, int len);
    int Send(string& data);
    std::unique_ptr<char>  ReadSome(int& bytes, int max);
    //try read more
    int ReadUntil(char* dest, size_t len, size_t max);
};


#endif