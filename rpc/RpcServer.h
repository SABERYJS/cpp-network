#ifndef MT_RPC_SERVER_H
#define MT_RPC_SERVER_H

#include"BaseTcpServer.h"
#include"RpcSession.h"
#include<map>
#include "HandlerProvider.h"
#include "RpcService.h"
#include<memory>
#include"CommonUtils.h"


using namespace std;

class RpcServer :public BaseTcpServer, public HandlerProvider {
private:
    std::map<std::string, std::shared_ptr<service_method_descriptor>>service_descriptors;
    std::list<std::shared_ptr<RpcService>>services;
public:
    RpcServer(string host, short port, AsyncDispatcher* dispatcher);
    ~RpcServer();
protected:
    virtual TcpSession* createSession()override;
public:
    void RegisterService(std::shared_ptr<RpcService>service);
    virtual shared_ptr<service_method_descriptor>QueryHandler(const string& method);
};

#endif