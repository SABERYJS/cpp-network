#ifndef MT_ECHO_SESSION_H
#define MT_ECHO_SESSION_H

#include"TcpSession.h"

class EchoSession :public TcpSession {
private:
    /* data */
public:
    EchoSession(/* args */) {}
    ~EchoSession() {}
protected:
    virtual void ProcessRead(const char* ptr, int size) override;
};

#endif