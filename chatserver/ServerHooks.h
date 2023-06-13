#ifndef MT_SERVER_HOOKS_H
#define MT_SERVER_HOOKS_H

#include<memory>
#include"TcpSession.h"
#include<map>
#include<mutex>
#include<stdexcept>
#include<spdlog/spdlog.h>
#include<fmt/format.h>
#include<google/protobuf/stubs/port.h>

using namespace std;


class ServerHooks {
private:
    std::mutex mtx;
    std::map<google::protobuf::int32, std::shared_ptr<TcpSession>>sessions;
public:
    ServerHooks() {}
    ~ServerHooks() {}
    bool NotifySessionAuthorized(std::shared_ptr<TcpSession>session, google::protobuf::int32 identity);

    void NotifySessionClosed(google::protobuf::int32 identity);

    std::shared_ptr<TcpSession>FindSession(google::protobuf::int32 identity);
};

#endif