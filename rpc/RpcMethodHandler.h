#ifndef MT_RPC_METHOD_HANDLER_H
#define MT_RPC_METHOD_HANDLER_H


#include<functional>
#include<type_traits>
#include<google/protobuf/message.h>
#include<cassert>
#include<spdlog/spdlog.h>

using namespace std;

class MethodArgs {
private:
    /* data */
public:
    MethodArgs(/* args */);
    ~MethodArgs();
};

enum Status {
    OK = 0,
    Failed = 1
};



class MethodHandler {
public:
    MethodHandler() {}
    ~MethodHandler() {}
    virtual Status ExecCall(void* req, std::function<void(google::protobuf::Message*)>scall) = 0;
    virtual void* Deserialize(const char* data, size_t len) = 0;
};

template<class RequestType, class ResponseType>
class RpcMethodHandler :public MethodHandler {
private:
    std::function<Status(RequestType*, ResponseType*)>fn_call;
public:
    RpcMethodHandler(std::function<Status(RequestType*, ResponseType*)>func) :fn_call(func) {
        static_assert(std::is_base_of<google::protobuf::Message, RequestType>::value, "RequestType must derived from google::protobuf::Message");
        static_assert(std::is_base_of<google::protobuf::Message, ResponseType>::value, "ResponseType must derived from google::protobuf::Message");
    }
    ~RpcMethodHandler() {}
    virtual Status ExecCall(void* req, std::function<void(google::protobuf::Message*)>scall)override {
        RequestType* request = static_cast<RequestType*>(req);
        ResponseType response;
        Status result = fn_call(request, &response);
        //free request object manual
        request->~RequestType();
        if (result == Status::OK) {
            scall(static_cast<google::protobuf::Message*>(&response));
        }
        return result;
    }

    virtual void* Deserialize(const char* data, size_t len)override {
        RequestType* request = new RequestType;
        if (!request->ParseFromArray(data, len)) {
            spdlog::error("parse protobuf failed");
            delete request;
            return nullptr;
        }
        return request;
    }
};

#endif