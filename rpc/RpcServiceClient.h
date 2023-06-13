#ifndef MT_RPC_SERVICE_CLIENT_H
#define MT_RPC_SERVICE_CLIENT_H

#include "BaseTcpClient.h"
#include "DynamicBytes.h"
#include<google/protobuf/message.h>
#include<mutex>
#include<cassert>
#include "CommonUtils.h"
#include<functional>
#include<vector>

template<class RequestType, class ResponseType>
class RpcRequest {
private:
public:
    RpcRequest(/* args */) {
        static_assert(std::is_base_of<google::protobuf::Message, RequestType>::value, "RequestType must derived from google::protobuf::Message");
        static_assert(std::is_base_of<google::protobuf::Message, ResponseType>::value, "ResponseType must derived from google::protobuf::Message");
    }
    ~RpcRequest() {}
    bool DoCall(DynamicBytes& buffer, const string& method, RequestType& request, ResponseType& response,
        std::function<bool(const char* data, size_t len)>DoWrite, std::function<int(char* dest, size_t bytes, size_t max)>DoRead) {
        buffer.Clear();
        int tlen = calc_len(method, request);
        buffer.Prepare(tlen);
        buffer.CData()[0] = static_cast<char>(method.size());
        copy(method.begin(), method.end(), buffer.CData() + 1);
        CommonUtils::EncodeInt16(buffer.CData() + 1 + method.size(), request.ByteSize());
        request.SerializeToArray(buffer.CData() + 3 + method.size(), request.ByteSize());
        buffer.Commit(tlen);
        if (!DoWrite(buffer.Data(), buffer.Size())) {
            return false;
        }
        buffer.Clear();

        while (true) {
            int bytes = 0;
            if ((bytes = DoRead(buffer.CData(), 2, buffer.CSize())) <= 0) {
                return false;
            }

            buffer.Commit(bytes);

            uint16_t payload_len = CommonUtils::DecodeInt16(buffer.Data());
            if (payload_len <= 0) {
                return false;
            }

            if ((buffer.Size() - 2) < payload_len) {
                continue;
            }
            if (!response.ParseFromArray(buffer.Data() + 2, payload_len)) {
                return false;
            }
            break;
        }

        return true;
    }
private:
    int calc_len(const string& method, RequestType& request) {
        return 3 + method.size() + request.ByteSize();
    }
};



class RpcServiceClient :public BaseTcpClient {
private:
    DynamicBytes buffer;
    std::mutex mtx_lock;
    std::vector<std::string>rpc_methods;
public:
    RpcServiceClient(string saddr, short sport);
    ~RpcServiceClient() {}
protected:
    template<class RequestType, class ResponseType>
    bool rpcCall(RequestType& request, ResponseType& response, int index) {
        std::lock_guard<std::mutex>guard(mtx_lock);
        RpcRequest< RequestType, ResponseType> req2;
        return req2.DoCall(buffer, rpc_methods[index], request, response, [this](const char* data, size_t len)->bool {
            return this->Send(data, len) == 0 ? true : false;
            }, [this](char* dest, size_t bytes, size_t max)->int {
                return ReadUntil(dest, bytes, max);
            });
    }
protected:
    void addRpcMethod(string method) {
        rpc_methods.push_back(method);
    }
};

#endif