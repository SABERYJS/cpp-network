#include "ChatServer.h"

#include "ChatServer.h"

ChatServer::ChatServer(string host, short port, AsyncDispatcher* dispatcher) :BaseTcpServer(host, port, dispatcher) {}
ChatServer::~ChatServer() {}
TcpSession* ChatServer::createSession() {
    return new ChatSession(&s_hooks);
}