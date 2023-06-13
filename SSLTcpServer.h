#ifndef MT_SSLTCP_SERVER_H
#define MT_SSLTCP_SERVER_H


#include "BaseTcpServer.h"

#include <openssl/ssl.h>
#include <openssl/err.h>
#include<string>
#include<stdexcept>
#include "SSLTcpSession.h"

using namespace std;


class SSLTcpServer :public BaseTcpServer {
private:
    string  server_certificate_key;
    string server_certificate_file;
    SSL_CTX* ssl_g_ctx;
public:
    SSLTcpServer(string host, short port, AsyncDispatcher* dispatcher, string key, string file);
    ~SSLTcpServer();
    void loadSSL();

protected:
    virtual TcpSession* createSession()override;
    inline SSL_CTX* get_ssl_ctx() { return ssl_g_ctx; }
};

#endif