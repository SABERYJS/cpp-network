#include "TcpSession.h"


TcpSession::TcpSession() {
    closed = false;
}

TcpSession::~TcpSession() {
    if (sockfd == 0) {
        //invalid session
        return;
    }
    dispatcher->RemoveTargetForever(sockfd);
    close(sockfd);
    //spdlog::info("tcpsession closed");
}

void TcpSession::Run(int sockfd, AsyncDispatcher* dispatcher) {
    spdlog::info("tcpsession run");
    this->sockfd = sockfd;
    this->dispatcher = dispatcher;
    doInit();
    startRead();
}

void TcpSession::startRead() {
    shared_ptr<TcpSession>obj = shared_from_this();
    dispatcher->SubscribeAsyncRead(sockfd, [obj](int error) {
        obj->readAsyncCallback(error);
        });
}

void TcpSession::startWrite() {
    shared_ptr<TcpSession>obj = shared_from_this();
    dispatcher->SubscribeAsyncWrite(sockfd, [obj](int error) {
        obj->writeAsyncCallback(error);
        });
}

void TcpSession::stopWrite() {
    assert(dispatcher->UnsubscribeAsyncWrite(sockfd) == 0);
}


void TcpSession::readAsyncCallback(int error) {
    std::lock_guard<std::mutex>guard(sync_mtx);
    if (closed) {
        return;
    }
    if (error) {
        handleError(error);
    }
    else {
        spdlog::info("readAsyncCallback");
        while (true) {
            int expect = db_read.CSize();
            int result = read(sockfd, db_read.CData(), expect);
            if (result < 0) {
                if (errno == EINTR) {
                    continue;
                }
                else if (errno == EAGAIN || errno == EWOULDBLOCK) {
                    //reference https://man7.org/linux/man-pages/man2/read.2.html
                    break;
                }
                else {
                    handleError(errno);
                    return;
                }
            }
            else if (result == 0) {
                //peer close connection
                handleError(0);
                return;
            }
            else {
                db_read.Commit(result);
                if (result == expect) {
                    //full,resize buffer
                    //100 can be changed
                    db_read.Prepare(db_read.MaxSize() * 2);
                }
                continue;
            }
        }

        ProcessRead(db_read.Data(), db_read.Size());
    }
}

void TcpSession::writeAsyncCallback(int error) {
    std::lock_guard<std::mutex>guard(sync_mtx);
    if (closed) {
        return;
    }
    if (error) {
        handleError(error);
    }
    else {
        notifyWriteCall();
        if (closed) {
            return;
        }
        if (db_write.Size() == 0) {
            stopWrite();
            return;
        }
        while (true) {
            int expect = db_write.Size();
            int result = write(sockfd, db_write.Data(), expect);
            if (result < 0) {
                if (errno == EINTR) {
                    continue;
                }
                else if (errno == EAGAIN || errno == EWOULDBLOCK) {
                    //reference https://man7.org/linux/man-pages/man2/read.2.html
                    break;
                }
                else {
                    handleError(errno);
                    return;
                }
            }
            else {
                db_write.Consume(result);
                if (db_write.Size() == 0) {
                    stopWrite();
                    break;
                }
                continue;
            }
        }
    }
}

void TcpSession::handleError(int error) {
    if (error != 0) {
        spdlog::error("tcp session error:{0:}", error);
    }
    closed = true;
    dispatcher->RemoveTargetSubscribe(sockfd);
    //hooks for derived class
    process_error(error);
}

void TcpSession::setConsumed(int size) {
    db_read.Consume(size);
}

void TcpSession::setCommitted(int size) {
    db_write.Commit(size);
}

void TcpSession::WriteData(string&& data) {
    WriteData(data.c_str(), data.size());
}

void TcpSession::WriteData(const char* ptr, size_t len) {
    std::lock_guard<std::mutex>guard(sync_mtx);
    db_write.Prepare(len);
    copy(ptr, ptr + len, db_write.CData());
    db_write.Commit(len);
    startWrite();
}

DynamicBytes& TcpSession::RefWriteDb(std::function<void(DynamicBytes& db)>ops) {
    ops(db_write);
    startWrite();
}


DynamicBytes& TcpSession::RefWriteDbMutex(std::function<void(DynamicBytes& db)>ops) {
    std::lock_guard<std::mutex>guard(sync_mtx);
    ops(db_write);
    startWrite();
}