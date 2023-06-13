#ifndef SSL_ECHO_SERVER_H
#define SSL_ECHO_SERVER_H


#include "SSLTcpServer.h"
#include "SSLEchoSession.h"


class SSLEchoServer :public SSLTcpServer {
private:
public:
    SSLEchoServer(string host, short port, AsyncDispatcher* dispatcher, string key, string file) :SSLTcpServer(host, port, dispatcher, key, file) {}
    ~SSLEchoServer() {}
protected:
    virtual TcpSession* createSession() override {
        return new SSLEchoSession(get_ssl_ctx());
    }
};


#endif