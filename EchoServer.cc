
#include "EchoServer.h"

TcpSession* EchoServer::createSession() {
    return  static_cast<TcpSession*>(new  EchoSession);
}