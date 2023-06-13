#include"SSLTcpSession.h"

SSLTcpSession::SSLTcpSession(SSL_CTX* ctx) :ssl_gctx(ctx) {}

SSLTcpSession::~SSLTcpSession() {
    //SSL_free() also calls the free()ing procedures for indirectly affected items, 
    //if applicable: the buffering BIO, the read and write BIOs, 
    //cipher lists specially created for this ssl, the SSL_SESSION. 
    //Do not explicitly free these indirectly freed up items before or after calling SSL_free(), 
    //as trying to free things twice may lead to program failure.
    SSL_free(ssl_context);
}

void SSLTcpSession::initSSL() {
    rbio = BIO_new(BIO_s_mem());
    wbio = BIO_new(BIO_s_mem());

    ssl_context = SSL_new(ssl_gctx);

    SSL_set_accept_state(ssl_context); /* sets ssl to work in server mode. */
    SSL_set_bio(ssl_context, rbio, wbio);
    //SSL_set_fd() sets the file descriptor fd as the input/output facility for the TLS/SSL (encrypted) side of ssl. 
    //fd will typically be the socket file descriptor of a network connection.
    //When performing the operation, a socket BIO is automatically created to interface between the ssl and fd.
    //The BIO and hence the SSL engine inherit the behaviour of fd.If fd is nonblocking, the ssl will also have nonblocking behaviour.
    //SSL_set_fd(ssl_context, get_fd());
}


void SSLTcpSession::doInit() {
    initSSL();
}

//reference https://gist.github.com/darrenjs/4645f115d10aa4b5cebf57483ec82eca
void SSLTcpSession::ProcessRead(const char* ptr, int size) {
    int consumed = 0;
    while (size > 0) {
        int result = 0;
        if ((result = BIO_write(rbio, ptr, size)) <= 0) {
            if (!BIO_should_retry(rbio)) {
                spdlog::error("BIO_write failed");
                handleError(0);
                return;
            }
            else {
                //retry later
                break;
            }
        }
        else {
            consumed += result;
            ptr += result;
            size -= result;
        }
    }

    setConsumed(consumed);

    if (!is_handshaked) {
        handshake();
    }

    if (!is_handshaked) {
        read_encrypted_data();
    }

    read_unencrypted_data();
}

void SSLTcpSession::handshake() {
    if (!SSL_is_init_finished(ssl_context)) {
        int code = SSL_accept(ssl_context);
        if (code == 0) {
            //The TLS/SSL handshake was not successful but was shut down controlled and by the specifications of the TLS/SSL protocol. 
            //Call SSL_get_error() with the return value ret to find out the reason.
            print_ssl_error(SSL_get_error(ssl_context, code));
            handleError(0);
            return;
        }
        else if (code == 1) {
            //The TLS/SSL handshake was successfully completed, a TLS/SSL connection has been established.
            is_handshaked = true;
        }
        else {
            //code<0
            //If the underlying BIO is nonblocking, SSL_accept() will also return when the 
                //underlying BIO could not satisfy the needs of SSL_accept() to continue the handshake, 
                //indicating the problem by the return value -1. In this case a call to SSL_get_error() 
                //with the return value of SSL_accept() will yield SSL_ERROR_WANT_READ or SSL_ERROR_WANT_WRITE.
                //The calling process then must repeat the call after taking appropriate action to satisfy the needs of SSL_accept(). 
                //The action depends on the underlying BIO.
            int error = SSL_get_error(ssl_context, code);
            if (error == SSL_ERROR_WANT_READ || error == SSL_ERROR_WANT_WRITE) {
                //print_ssl_error(error);
                return;
            }
            else {
                print_ssl_error(error);
                handleError(0);
            }
        }
    }
}

void SSLTcpSession::read_unencrypted_data() {
    if (check_closed()) {
        return;
    }
    if (!is_handshaked) {
        return;
    }
    while (true) {
        int result;
        int expect = ssl_readbuf.CSize();
        result = SSL_read(ssl_context, ssl_readbuf.CData(), expect);
        if (result > 0) {
            //The read operation was successful. The return value is the number of bytes actually read from the TLS/SSL connection.
            ssl_readbuf.Commit(result);
            if (result < expect) {
                break;
            }
            else {
                //try read 
                continue;
            }
        }
        else {
            //The read operation was not successful, because either the connection was closed, 
            //an error occurred or action must be taken by the calling process. Call SSL_get_error(3) with the return value ret to find out the reason.
            //If the underlying BIO is nonblocking, a read function will also return 
            //when the underlying BIO could not satisfy the needs of the function to continue the operation. 
            //In this case a call to SSL_get_error(3) with the return value of the read function will yield SSL_ERROR_WANT_READ or SSL_ERROR_WANT_WRITE. 
            int error = SSL_get_error(ssl_context, result);
            if (error == SSL_ERROR_WANT_READ || error == SSL_ERROR_WANT_WRITE) {
                break;
            }
            else {
                print_ssl_error(error);
                handleError(0);
                break;
            }
        }
    }
    if (!check_closed() && ssl_readbuf.Size() > 0) {
        ProcessSSLData(ssl_readbuf.Data(), ssl_readbuf.Size());
    }
}

void SSLTcpSession::SetSSLDataConsumed(int len) {
    ssl_readbuf.Consume(len);
}

void SSLTcpSession::WriteSSLData(const char* ptr, size_t len) {
    ssl_writebuf.Prepare(len);
    copy(ptr, ptr + len, ssl_writebuf.CData());
    ssl_writebuf.Commit(len);
    try_write_sslbuf_data();
}

void SSLTcpSession::read_encrypted_data() {
    //read encrypted data from ssl object,then write this to socket buffer
#ifndef TEMP_BIO_READ_BUFFER_SIZE
#define TEMP_BIO_READ_BUFFER_SIZE 1024
#endif
    char buf[TEMP_BIO_READ_BUFFER_SIZE];
    while (true) {
        int result = BIO_read(wbio, buf, TEMP_BIO_READ_BUFFER_SIZE);
        //A 0 or -1 return is not necessarily an indication of an error. 
        //In particular when the source/sink is non-blocking or of a certain type it may merely be an indication 
        //that no data is currently available and that the application should retry the operation later.
        //All these functions return either the amount of data successfully read or written
        // (if the return value is positive) or that no data was successfully read or written if the result is 0 or -1. 
        //If the return value is -2 then the operation is not implemented in the specific BIO type.
        if (result > 0) {
            WriteData(buf, result);
            continue;
        }
        else {
            if (!BIO_should_retry(wbio)) {
                print_ssl_error(SSL_get_error(ssl_context, result));
                handleError(0);
                break;
            }
            else {
                break;
            }
        }
    }
}

void SSLTcpSession::try_write_sslbuf_data() {
    //firstly, write unencrypted data to ssl object
    while (ssl_writebuf.Size() > 0) {
        //disable SSL_MODE_ENABLE_PARTIAL_WRITE 
        //If the underlying BIO is nonblocking the write functions will also return 
        //when the underlying BIO could not satisfy the needs of the function to continue the operation. 
        //In this case a call to SSL_get_error(3) with the return value of 
        //the write function will yield SSL_ERROR_WANT_READ or SSL_ERROR_WANT_WRITE
        //The write functions will only return with success when the complete contents of buf of length num has been written.
        int result = SSL_write(ssl_context, ssl_writebuf.Data(), ssl_writebuf.Size());
        if (result > 0) {
            //The write operation was successful, the return value is the number of bytes actually written to the TLS/SSL connection.
            ssl_writebuf.Consume(result);
            continue;
        }
        else {
            int error = SSL_get_error(ssl_context, result);
            if (error == SSL_ERROR_WANT_READ || error == SSL_ERROR_WANT_WRITE) {
                break;
            }
            else {
                print_ssl_error(error);
                handleError(0);
                break;
            }
        }
    }
    read_encrypted_data();
}

void SSLTcpSession::notifyWriteCall() {
    try_write_sslbuf_data();
}

void SSLTcpSession::WriteSSLData(string&& data) {
    WriteSSLData(data.c_str(), data.size());
}

void SSLTcpSession::print_ssl_error(int error) {
    char buffer[256] = { '\0' };
    ERR_error_string(error, buffer);
    spdlog::error("ssl error,{}", buffer);
}