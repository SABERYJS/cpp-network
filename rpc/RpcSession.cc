#include "RpcSession.h"

RpcSession::RpcSession(HandlerProvider* provider) :hprovider(provider) {}

RpcSession::~RpcSession() {}

//1byte(length of method)+2bytes(protobuf overload)
void RpcSession::ProcessRead(const char* ptr, int size) {
    int used = 0;
    int consumed = 0;
    while (true) {
        if ((size - used) < 1) {
            return;
        }
        uint8_t method_len = static_cast<uint8_t>(ptr[0]);
        if (method_len <= 0) {
            handleError(0);
            return;
        }
        ptr += 1;
        used += 1;
        if ((size - used) < method_len) {
            return;
        }
        string method(ptr, method_len);
        ptr += method_len;
        used += method_len;
        if ((size - used) < 2) {
            return;
        }
        uint16_t payload_len = CommonUtils::DecodeInt16(ptr);
        if (payload_len <= 0) {
            handleError(0);
            return;
        }
        ptr += 2;
        used += 2;
        if ((size - used) < payload_len) {
            return;
        }
        //enough for handler
        std::shared_ptr<service_method_descriptor> descriptor = hprovider->QueryHandler(method);
        if (descriptor == nullptr) {
            handleError(0);
            return;
        }
        void* request = descriptor->handler->Deserialize(ptr, payload_len);
        if (request == nullptr) {
            handleError(0);
            return;
        }
        ptr += payload_len;
        used += payload_len;
        Status result = descriptor->handler->ExecCall(request, [this](google::protobuf::Message* mproto) {
            this->RefWriteDb([&mproto](DynamicBytes& buffer) {
                int size = mproto->ByteSize() + 2;
                buffer.Prepare(size);
                CommonUtils::EncodeInt16(buffer.CData(), mproto->ByteSize());
                mproto->SerializeToArray(buffer.CData() + 2, mproto->ByteSize());
                buffer.Commit(size);
                });
            });
        if (result != Status::OK) {
            handleError(0);
            return;
        }
        consumed += 3 + payload_len + method_len;
    }
    //call once
    setConsumed(consumed);
}