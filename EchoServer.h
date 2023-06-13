#ifndef MT_ECHO_SERVER_H
#define MT_ECHO_SERVER_H

#include "BaseTcpServer.h"
#include "EchoSession.h"


class EchoServer :public BaseTcpServer {
private:
    /* data */
public:
    EchoServer(string host, short port, AsyncDispatcher* dispatcher) :BaseTcpServer(host, port, dispatcher) {}
    ~EchoServer() {}
protected:
    virtual TcpSession* createSession()override;
};


#endif