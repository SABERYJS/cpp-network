#ifndef MT_CHAT_SESSION_H
#define MT_CHAT_SESSION_H

#include "TcpSession.h"
#include "ProtobufAssembler.h"
#include "protocompiled/protocols.pb.h"
#include "ServerHooks.h"

class ServerHooks;

class ChatSession :public TcpSession {
private:
    ProtobufAssembler<rpc::Message> assembler;
    ServerHooks* s_hooks;
    google::protobuf::int32 identity;
public:
    ChatSession(ServerHooks* hook);
    ~ChatSession();
    void SendMessage(rpc::Message& message);
    //reference RefWriteDbMutex for usage
    void SendMessageMutex(rpc::Message& message);
protected:
    virtual void ProcessRead(const char* ptr, int size)override;
    virtual void process_error(int error)override;
private:
    void handle_auth_message(const rpc::RequestAuth& auth);
    void handle_peer_chat_message(const rpc::SendChatMsg& chat);
    void SendMessageToPeer(rpc::Message& message, google::protobuf::int32 peer);
};

#endif