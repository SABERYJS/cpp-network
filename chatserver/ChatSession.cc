#include "ChatSession.h"

ChatSession::ChatSession(ServerHooks* hook) :s_hooks(hook) {}
ChatSession::~ChatSession() {}

void ChatSession::ProcessRead(const char* ptr, int size) {
    int consumed = 0;
    while (consumed < size) {
        rpc::Message message;
        int bytes = assembler.Decode(ptr + consumed, size - consumed, message);
        if (bytes < 0) {
            handleError(0);
            return;
        }
        if (bytes == 0) {
            //no enough bytes
            return;
        }
        if (message.mtype() == rpc::MessageType::MessageTypeAuth) {
            handle_auth_message(message.rauth());
        }
        else if (message.mtype() == rpc::MessageType::MessageTypeChatMsg) {
            handle_peer_chat_message(message.smsg());
        }
        consumed += bytes;
    }
    setConsumed(consumed);
}

void ChatSession::handle_auth_message(const rpc::RequestAuth& auth) {
    if (!s_hooks->NotifySessionAuthorized(shared_from_this(), auth.uid())) {
        handleError(0);
        return;
    }
    identity = auth.uid();
    spdlog::info("auth received ,{}:{}", auth.uid(), auth.password());
}

void ChatSession::handle_peer_chat_message(const rpc::SendChatMsg& chat) {
    spdlog::info("chat message,{},{}", chat.peer(), chat.msgtype());
    rpc::Message cmsg;
    cmsg.set_mtype(rpc::MessageType::MessageTypeChatMsg);
    cmsg.mutable_smsg()->set_peer(identity);
    cmsg.mutable_smsg()->set_msgtype(chat.msgtype());
    if (chat.msgtype() == rpc::ChatMsgType::ChatMsgTypeText) {
        cmsg.mutable_smsg()->set_text(chat.text());
    }
    else if (chat.msgtype() == rpc::ChatMsgType::ChatMsgTypePic) {
        cmsg.mutable_smsg()->set_picurl(chat.picurl());
    }
    else if (chat.msgtype() == rpc::ChatMsgType::ChatMsgTypeVideo) {
        cmsg.mutable_smsg()->set_videourl(chat.videourl());
    }
    else if (chat.msgtype() == rpc::ChatMsgType::ChatMsgTypeVoice) {
        cmsg.mutable_smsg()->set_voiceurl(chat.voiceurl());
    }
    else {
        return;
    }
    SendMessageToPeer(cmsg, chat.peer());
}

void ChatSession::SendMessage(rpc::Message& message) {
    RefWriteDb([&message, this](DynamicBytes& db) {
        this->assembler.Encode(message, db);
        });
}

void ChatSession::SendMessageMutex(rpc::Message& message) {
    RefWriteDbMutex([&message, this](DynamicBytes& db) {
        this->assembler.Encode(message, db);
        });
}


void ChatSession::SendMessageToPeer(rpc::Message& message, google::protobuf::int32 peer) {
    auto session = s_hooks->FindSession(peer);
    if (session == nullptr) {
        spdlog::info("peer {} offline", peer);
        return;
    }
    ChatSession* cs = reinterpret_cast<ChatSession*>(session.get());
    cs->SendMessageMutex(message);
}

void ChatSession::process_error(int error) {
    s_hooks->NotifySessionClosed(identity);
}