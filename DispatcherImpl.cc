#include "DispatcherImpl.h"

int DispatcherImpl::Create() {
    cfd = epoll_create1(0);
    if (cfd <= 0) {
        throw std::runtime_error(fmt::format("epoll_create1 failed,{}", strerror(errno)));
    }
    //spdlog::info("CreateAsyncFd {}", epfd);
    return cfd;
}


DispatcherImpl::~DispatcherImpl() {
    close(cfd);
}

int DispatcherImpl::Add(int fd, epoll_event* evt) {
    return epoll_ctl(cfd, EPOLL_CTL_ADD, fd, evt);
}
int DispatcherImpl::Modify(int fd, epoll_event* evt) {
    return epoll_ctl(cfd, EPOLL_CTL_MOD, fd, evt);
}
int DispatcherImpl::Delete(int fd, epoll_event* evt) {
    return epoll_ctl(cfd, EPOLL_CTL_DEL, fd, evt);
}
void DispatcherImpl::Wait() {
    epoll_event events[DISPATCHER_WAIT_COUNT];
    while (true) {
        int ret = epoll_wait(cfd, events, DISPATCHER_WAIT_COUNT, -1);
        if (ret < 0) {
            if (errno == EINTR) {
                continue;
            }
            else {
                spdlog::error("epoll_wait fatal error,{}", strerror(errno));
                exit(1);
            }
        }
        else {
            for (int i = 0;i < ret;i++) {
                epoll_event* e = &events[i];
                Subcriber* suber = static_cast<Subcriber*>(e->data.ptr);
                if (suber == nullptr) {
                    continue;
                }

                int flags = e->events;

                auto handlcall = [suber, flags]() {
                    auto lock = suber->auto_ptr();
                    if (suber->recycled) {
                        return;
                    }
                    int error = 0;
                    if (flags & (EPOLLERR | EPOLLRDHUP | EPOLLHUP)) {
                        if (flags & EPOLLERR) {
                            error = EPOLLERR;
                        }
                        else if (flags & EPOLLRDHUP) {
                            //EPOLLHUP means that the peer closed their end of the connection.
                            error = EPOLLRDHUP;
                        }
                        else {
                            error = EPOLLHUP;
                            //You use EPOLLRDHUP to detect peer shutdown, not EPOLLHUP (which signals an unexpected close of the socket, 
                            //i.e. usually an internal error).
                        }
                    }

                    if (flags & EPOLLIN && suber->pass()) {
                        suber->callr(error);
                    }
                    if (flags & EPOLLOUT && suber->pass()) {
                        suber->callw(error);
                    }
                    if (!suber->pass()) {
                        suber->do_clear();
                    }
                };
                tPool->Async(handlcall);
            }
        }
    }
}