#include "BaseTcpServer.h"


BaseTcpServer::~BaseTcpServer() {
    dispatcher->RemoveTargetSubscribe(sockfd);
    dispatcher->RemoveTargetForever(sockfd);
    close(sockfd);
    spdlog::error("server quit");
}

int   BaseTcpServer::Socket() {
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    return sockfd;
}

int BaseTcpServer::Bind() {
    saddr.sin_family = AF_INET;
    if (inet_pton(AF_INET, host.c_str(), &(saddr.sin_addr)) < 0) {
        return -1;
    }
    saddr.sin_port = htons(port);
    return bind(sockfd, (sockaddr*)&saddr, sizeof(sockaddr_in));
}
int BaseTcpServer::Listen() {
    return listen(sockfd, MT_TCP_SERVER_BACKLOG);
}
void BaseTcpServer::Accept(int error) {
    std::lock_guard<mutex>guard(mtx);
    if (error) {
        spdlog::error("server accept failed,{}", strerror(errno));
        exit(1);
    }
    while (true) {
        shared_ptr<TcpSession>session(createSession());
        socklen_t slen = sizeof(sockaddr_in);
        int clientfd;
        if ((clientfd = accept(sockfd, reinterpret_cast<sockaddr*>(session->Saddr()), &slen)) <= 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                //no more connections
                spdlog::info("EAGAIN or EWOULDBLOCK");
                break;
            }
            else if (errno == EINTR) {
                continue;
            }
            else {
                spdlog::error("accept failed,{},{}", strerror(errno), errno);
            }
        }
        else {
            spdlog::info("accept success");
            if (CommonUtils::SetFileDescriptorNonBlock(clientfd) == 0) {
                session->Run(clientfd, dispatcher);
            }
            else {
                //syscall failed
            }
        }
    }

}

void  BaseTcpServer::Run() {
    if (Socket() <= 0 || SetOptions() < 0 || Bind() < 0 || Listen() < 0) {
        spdlog::error("run failed,{}", strerror(errno));
        throw std::runtime_error("start server failed");
        return;
    }
    if (dispatcher->SubscribeAsyncRead(sockfd, std::bind(&BaseTcpServer::Accept, this, placeholders::_1)) < 0) {
        spdlog::error("tcp server SubscribeAsyncRead failed,{}", strerror(errno));
        throw std::runtime_error("SubscribeAsyncRead failed");
    }
}

int BaseTcpServer::SetSocketReUseAddr() {
    return  CommonUtils::SetSocketOption(sockfd, SO_REUSEADDR);
}

int BaseTcpServer::SetSocketReUsePort() {
    return  CommonUtils::SetSocketOption(sockfd, SO_REUSEPORT);
}

int BaseTcpServer::SetOptions() {
    return (SetSocketReUsePort() < 0 || SetSocketReUseAddr() < 0 || CommonUtils::SetFileDescriptorNonBlock(sockfd) < 0) ? -1 : 0;
}