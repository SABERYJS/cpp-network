#ifndef MT_SSL_ECHO_SESSION_H
#define MT_SSL_ECHO_SESSION_H


#include "SSLTcpSession.h"
#include<fmt/format.h>


class SSLEchoSession :public SSLTcpSession {
public:
    SSLEchoSession(SSL_CTX* ctx) :SSLTcpSession(ctx) {}
    ~SSLEchoSession() {}
protected:
    virtual void ProcessSSLData(const char* ptr, int size)override {
        spdlog::info("ProcessSSLData:{}", string(ptr, size));
        SetSSLDataConsumed(size);
        WriteSSLData(fmt::format("{}-resp", string(ptr, size)));
    }
};


#endif