#include "SSLTcpServer.h"

void SSLTcpServer::loadSSL() {
    SSL_library_init();
    OpenSSL_add_all_algorithms();
    SSL_load_error_strings();
    //ERR_load_BIO_strings();
    ERR_load_crypto_strings();

    /* create the SSL server context */
    ssl_g_ctx = SSL_CTX_new(SSLv23_server_method());
    if (!ssl_g_ctx)
        throw runtime_error("SSL_CTX_new failed");

    /* Load certificate and private key files, and check consistency  */
    int err;
    err = SSL_CTX_use_certificate_file(ssl_g_ctx, server_certificate_file.c_str(), SSL_FILETYPE_PEM);
    if (err != 1)
        throw runtime_error("SSL_CTX_use_certificate_file failed");
    else
        spdlog::info("load certificate file success");

    /* Indicate the key file to be used */
    err = SSL_CTX_use_PrivateKey_file(ssl_g_ctx, server_certificate_key.c_str(), SSL_FILETYPE_PEM);
    if (err != 1)
        throw runtime_error("SSL_CTX_use_PrivateKey_file failed");
    else
        spdlog::info("load certificate key success");

    /* Make sure the key and certificate file match. */
    if (SSL_CTX_check_private_key(ssl_g_ctx) != 1)
        throw runtime_error("SSL_CTX_check_private_key failed");
    else
        spdlog::info("check certificate key valid");

    /* Recommended to avoid SSLv2 & SSLv3 */
    SSL_CTX_set_options(ssl_g_ctx, SSL_OP_ALL | SSL_OP_NO_SSLv2 | SSL_OP_NO_SSLv3);

    //long mode = SSL_CTX_get_mode(ssl_g_ctx);
    //Allow SSL_write_ex(..., n, &r) to return with 0 < r < n (i.e. report success when just a single record has been written). 
    //This works in a similar way for SSL_write(). When not set (the default),
    //SSL_write_ex() or SSL_write() will only report success once the complete chunk was written. 
    //Once SSL_write_ex() or SSL_write() returns successful, r bytes have been written and the next call to SSL_write_ex() 
    //or SSL_write() must only send the n-r bytes left, imitating the behaviour of write().
    //SSL_CTX_set_mode(ssl_g_ctx, mode | SSL_MODE_ENABLE_PARTIAL_WRITE);
    spdlog::info("ssl init done");
}


TcpSession* SSLTcpServer::createSession() {
    throw runtime_error("SSLTcpServer::createSession must be implemented");
}

SSLTcpServer::SSLTcpServer(string host, short port, AsyncDispatcher* dispatcher, string key, string file) :
    BaseTcpServer(host, port, dispatcher), server_certificate_key(key), server_certificate_file(file) {
    loadSSL();
}
SSLTcpServer::~SSLTcpServer() {}