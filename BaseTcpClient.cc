#include "BaseTcpClient.h"


BaseTcpClient::BaseTcpClient(string saddr, short sport) :server_addr(saddr), server_port(sport) {
    createSocket();
}

BaseTcpClient::~BaseTcpClient() {
    close(sockfd);
}

void BaseTcpClient::createSocket() {
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd <= 0) {
        throw std::runtime_error("sock failed");
    }
}


void BaseTcpClient::Connect() {
    saddr.sin_family = AF_INET;
    saddr.sin_port = htons(server_port);
    if (inet_pton(AF_INET, server_addr.c_str(), &saddr.sin_addr) <= 0) {
        throw std::runtime_error("inet_pton failed");
    }
    if (connect(sockfd, reinterpret_cast<sockaddr*>(&saddr), sizeof(saddr)) < 0) {
        throw std::runtime_error("connect failed");
    }
}

int BaseTcpClient::write_base(const char* data, int len) {
    while (true) {
        int result = write(sockfd, data, len);
        if (result < 0) {
            if (errno == EINTR) {
                continue;
            }
            else {
                return -1;
            }
        }
        else {
            len -= result;
            data += result;
            if (len == 0) {
                return 0;
            }
        }
    }
}

int BaseTcpClient::read_base(char* dest, int len) {
    while (true) {
        int result = read(sockfd, dest, len);
        if (result < 0) {
            if (errno == EINTR) {
                continue;
            }
            else {
                return -1;
            }
        }
        else if (result == 0) {
            return 0;
        }
        else {
            return result;
        }
    }
}

int BaseTcpClient::Send(const char* data, int len) {
    return write_base(data, len);
}

int BaseTcpClient::Send(string& data) {
    return Send(data.c_str(), data.size());
}

std::unique_ptr<char>  BaseTcpClient::ReadSome(int& bytes, int max) {
    std::unique_ptr<char>ptr(new char[max]);
    if ((bytes = read_base(ptr.get(), max)) < 0) {
        throw std::runtime_error("read failed");
    }
    return ptr;
}

int BaseTcpClient::ReadUntil(char* dest, size_t len, size_t max) {
    int bytes = 0;
    int ret = 0;
    while (bytes < len) {
        ret = read_base(dest + bytes, max - bytes);
        if (ret < 0) {
            throw std::runtime_error("unexpected error failded");
        }
        else if (ret == 0) {
            throw std::runtime_error("server closed");
        }
        bytes += ret;
    }
    return bytes;
}