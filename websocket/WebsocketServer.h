#ifndef MT_WEBSOCKET_SERVER_H
#define MT_WEBSOCKET_SERVER_H

#include "BaseTcpServer.h"
#include "WebsocketSession.h"


class WebsocketServer :public BaseTcpServer {
private:
    /* data */
public:
    WebsocketServer(string host, short port, AsyncDispatcher* dispatcher);
    ~WebsocketServer();
protected:
    virtual TcpSession* createSession()override;
};


#endif