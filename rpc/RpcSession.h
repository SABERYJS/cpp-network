#ifndef MT_RPC_SESSION_H
#define MT_RPC_SESSION_H

#include "TcpSession.h"
#include "HandlerProvider.h"
#include <cctype>
#include<string>
#include<google/protobuf/message.h>
#include<functional>
#include"DynamicBytes.h"
#include"RpcMethodHandler.h"
#include"CommonUtils.h"


using namespace std;

class RpcSession :public  TcpSession {
private:
    HandlerProvider* hprovider;
public:
    RpcSession(HandlerProvider* provider);
    ~RpcSession();
protected:
    virtual void ProcessRead(const char* ptr, int size)override;
};

#endif