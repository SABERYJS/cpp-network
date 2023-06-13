#ifndef MT_WEBSOCKET_EXCEPTION_H
#define MT_WEBSOCKET_EXCEPTION_H

#include<exception>
#include<stdexcept>
#include<string>
#include<fmt/format.h>

using namespace std;

class WebsocketException :public std::exception {
private:
    string error_desc{};
public:
    WebsocketException(const char* error) :error_desc(std::move(fmt::format("websocket exception {}", error))) {}
    ~WebsocketException() {}
    virtual const char*
        what() const noexcept {
        return error_desc.c_str();
    }
};

#endif