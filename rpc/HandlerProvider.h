#ifndef MT_HANDLER_PROVIDER_H
#define MT_HANDLER_PROVIDER_H

#include"RpcService.h"

using namespace std;

//pure interface
class HandlerProvider {
public:
    virtual shared_ptr<service_method_descriptor>QueryHandler(const string& method) = 0;
};
#endif