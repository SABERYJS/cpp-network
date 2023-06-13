#ifndef MT_RPC_SERVICE_H
#define MT_RPC_SERVICE_H

#include<list>
#include<functional>
#include<memory>
#include<map>
#include"RpcMethodHandler.h"

using namespace std;

typedef std::function<void()> DescriptorMethod;


struct service_method_descriptor {
    string method_name;
    MethodHandler* handler;
    service_method_descriptor(string method, MethodHandler* hdr) :method_name(method), handler(hdr) {}
};

//base service class for all service 
class RpcService {
private:
    std::map<std::string, shared_ptr<service_method_descriptor>>descriptors;
public:
    RpcService(/* args */);
    ~RpcService();
    std::map<std::string, shared_ptr<service_method_descriptor>>& GetDescriptors();
protected:
    void RegisterDescriptors(const string& method, std::shared_ptr<service_method_descriptor>descriptor);
};

#endif