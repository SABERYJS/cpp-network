#ifndef MT_SSL_TCP_SESSION_H
#define MT_SSL_TCP_SESSION_H

#include <openssl/ssl.h>
#include <openssl/err.h>
#include "TcpSession.h"
#include "DynamicBytes.h"



class SSLTcpSession :public TcpSession {
private:
    SSL* ssl_context;
    BIO* rbio;
    BIO* wbio;
    bool is_handshaked{ false };
    DynamicBytes ssl_readbuf;
    DynamicBytes ssl_writebuf;
    SSL_CTX* ssl_gctx;
public:
    SSLTcpSession(SSL_CTX* ctx);
    ~SSLTcpSession();
private:
    void initSSL();
    void handshake();
    void print_ssl_error(int error);
    void read_unencrypted_data();
    void read_encrypted_data();
    void try_write_sslbuf_data();
protected:
    virtual void doInit()override;
    virtual void ProcessRead(const char* ptr, int size)override;
    virtual void ProcessSSLData(const char* ptr, int size) = 0;
    void SetSSLDataConsumed(int len);
    void WriteSSLData(const char* ptr, size_t len);
    void WriteSSLData(string&& data);
    virtual void notifyWriteCall() override;
};

#endif