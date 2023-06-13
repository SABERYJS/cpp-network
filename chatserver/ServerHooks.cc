#include "ServerHooks.h"

bool ServerHooks::NotifySessionAuthorized(std::shared_ptr<TcpSession>session, google::protobuf::int32 identity) {
    std::lock_guard<std::mutex>guard(mtx);
    if (sessions.find(identity) != sessions.end()) {
        return false;
    }
    sessions[identity] = session;
    return true;
}

void ServerHooks::NotifySessionClosed(google::protobuf::int32 identity) {
    std::lock_guard<std::mutex>guard(mtx);
    if (sessions.find(identity) == sessions.end()) {
        throw std::runtime_error(fmt::format("session {} closed", identity));
    }
    sessions.erase(identity);
}

std::shared_ptr<TcpSession> ServerHooks::FindSession(google::protobuf::int32 identity) {
    std::lock_guard<std::mutex>guard(mtx);
    if (sessions.find(identity) != sessions.end()) {
        return sessions.find(identity)->second;
    }
    else {
        return nullptr;
    }
}