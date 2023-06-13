#include "WebsocketServer.h"

WebsocketServer::WebsocketServer(string host, short port, AsyncDispatcher* dispatcher) :BaseTcpServer(host, port, dispatcher) {}
WebsocketServer::~WebsocketServer() {}

TcpSession* WebsocketServer::createSession() {
    return  static_cast<WebsocketSession*>(new  WebsocketSession);
}