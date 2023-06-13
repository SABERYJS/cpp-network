#ifndef MT_COMMON_UTILS_H
#define MT_COMMON_UTILS_H

#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>
#include<thread>
#include<string>
#include<algorithm>
#include<cctype>
#include<sys/types.h>
#include<sys/stat.h>
#include<csignal>
#include<cstdint>
#include<stdexcept>
#include<spdlog/spdlog.h>
#include<fmt/format.h>
#include<errno.h>
#include<error.h>
#include "CharArray.h"

using namespace std;


static const string alphabets{"0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz"};

class CommonUtils {
public:
    CommonUtils(/* args */) = delete;
    ~CommonUtils() = delete;
    static int SetSocketOption(int sockfd, int options);
    static int SetFileDescriptorNonBlock(int fd);
    static int GetSystemCpuCores();
    static bool CheckFileExist(string path);
    static bool CheckFileReadPermission(string path);
    static bool CheckFileWritePermission(string path);
    static bool CheckFileExecPermission(string path);
    static void ToLowercase(string& s);
    static string GetRandomSequence(int len);
    static void TrimLeft(std::string& s, const char* chs);
    static void TrimRight(std::string& s, const char* chs);
    static void Trim(std::string& s, const char* chs);
    static bool CompareIgnoreCase(string&& a, string&& b);
    //reference https://stackoverflow.com/questions/17954432/creating-a-daemon-in-linux/17955149#17955149
    static int  Daemon(string workdir);
    static void EncodeInt32(char* data, uint32_t value);
    static uint32_t DecodeInt32(const char* data);
    static void EncodeInt16(char* data, uint16_t value);
    static uint16_t DecodeInt16(const char* data);
    static int GetFileSize(const string& file);
    static std::shared_ptr<CharArray> ReadFileAll(const string& file);
};


#endif