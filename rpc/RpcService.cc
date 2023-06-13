#include "RpcService.h"

RpcService::RpcService(/* args */) {}
RpcService::~RpcService() {}

std::map<std::string, shared_ptr<service_method_descriptor>>& RpcService::GetDescriptors() {
    return descriptors;
}

void RpcService::RegisterDescriptors(const string& method, std::shared_ptr<service_method_descriptor>descriptor) {
    descriptors[method] = descriptor;
}