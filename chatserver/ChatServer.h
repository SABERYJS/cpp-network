#ifndef CHAT_SERVER_H
#define CHAT_SERVER_H

#include "BaseTcpServer.h"
#include "ChatSession.h"
#include "ServerHooks.h"



class ChatServer :public BaseTcpServer
{
private:
    ServerHooks s_hooks;
public:
    ChatServer(string host, short port, AsyncDispatcher* dispatcher);
    ~ChatServer();
protected:
    virtual TcpSession* createSession()override;
};


#endif