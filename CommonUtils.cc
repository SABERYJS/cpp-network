#include "CommonUtils.h"

int CommonUtils::SetSocketOption(int sockfd, int options) {
    int opt;
    return setsockopt(sockfd, SOL_SOCKET,
        options, &opt,
        sizeof(opt));
}

int CommonUtils::SetFileDescriptorNonBlock(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1) return -1;
    flags = flags | O_NONBLOCK;
    return fcntl(fd, F_SETFL, flags);
}

int CommonUtils::GetSystemCpuCores() {
    return std::thread::hardware_concurrency();
}

bool CommonUtils::CheckFileExist(string path) {
    return access(path.c_str(), F_OK) == 0;
}

bool CommonUtils::CheckFileReadPermission(string path) {
    return access(path.c_str(), R_OK) == 0;
}

bool CommonUtils::CheckFileWritePermission(string path) {
    return access(path.c_str(), W_OK) == 0;
}

bool CommonUtils::CheckFileExecPermission(string path) {
    return access(path.c_str(), X_OK) == 0;
}

void CommonUtils::ToLowercase(string& data) {
    std::transform(data.begin(), data.end(), data.begin(),
        [](unsigned char c) { return std::tolower(c); });
}

string CommonUtils::GetRandomSequence(int len) {
    srand(time(NULL));
    if (len == 0) {
        return "";
    }
    string dest;
    dest.reserve(len);
    while (len-- > 0) {
        dest += alphabets[rand() % alphabets.size()];
    }
    return dest;
}

void CommonUtils::TrimLeft(std::string& s, const char* chs) {
    s.erase(0, s.find_first_not_of(chs));
}

void CommonUtils::TrimRight(std::string& s, const char* chs) {
    s.erase(s.find_last_not_of(chs) + 1);
}

void CommonUtils::Trim(std::string& s, const char* chs) {
    TrimRight(s, chs);
    TrimLeft(s, chs);
}

bool CommonUtils::CompareIgnoreCase(string&& a, string&& b) {
    return a.size() == b.size() &&
        std::equal(a.begin(), a.end(), b.begin(), b.end(), [](char c1, char c2) {
        if (c1 == c2)
            return true;
        else if (std::toupper(c1) == std::toupper(c2))
            return true;
        return false;
            });
}


int CommonUtils::Daemon(string workdir) {
    if (!CommonUtils::CheckFileExist(workdir)) {
        throw std::runtime_error(fmt::format("dir {} not accessable", workdir));
    }
    pid_t pid;

    /* Fork off the parent process */
    pid = fork();

    /* An error occurred */
    if (pid < 0)
        exit(EXIT_FAILURE);

    /* Success: Let the parent terminate */
    if (pid > 0)
        exit(EXIT_SUCCESS);

    /* On success: The child process becomes session leader */
    if (setsid() < 0)
        exit(EXIT_FAILURE);

    /* Catch, ignore and handle signals */
    //TODO: Implement a working signal handler */
    signal(SIGCHLD, SIG_IGN);
    signal(SIGHUP, SIG_IGN);

    /* Fork off for the second time*/
    pid = fork();

    /* An error occurred */
    if (pid < 0)
        exit(EXIT_FAILURE);

    /* Success: Let the parent terminate */
    if (pid > 0)
        exit(EXIT_SUCCESS);

    /* Set new file permissions */
    umask(0);

    /* Change the working directory to the root directory */
    /* or another appropriated directory */
    chdir(workdir.c_str());

    /* Close all open file descriptors */
    // int x;
    // for (x = sysconf(_SC_OPEN_MAX); x >= 0; x--) {
    //     close(x);
    // }
}


void CommonUtils::EncodeInt32(char* data, uint32_t value) {
    data[0] = static_cast<uint8_t>(value >> 24 & 0x000000ff);
    data[1] = static_cast<uint8_t>(value >> 16 & 0x000000ff);
    data[2] = static_cast<uint8_t>(value >> 8 & 0x000000ff);
    data[3] = static_cast<uint8_t>(value & 0x000000ff);
}

uint32_t CommonUtils::DecodeInt32(const char* data) {
    uint32_t result = (static_cast<uint32_t>(data[0]) << 24 & 0xff000000)
        | static_cast<uint32_t>(data[1]) << 16 & 0x00ff0000
        | static_cast<uint32_t>(data[2]) << 8 & 0x0000ff00
        | static_cast<uint32_t>(data[3]) & 0x000000ff;
    return result;
}

void CommonUtils::EncodeInt16(char* data, uint16_t value) {
    data[0] = static_cast<uint8_t>(value >> 8 & 0x00ff);
    data[1] = static_cast<uint8_t>(value & 0x00ff);
}

uint16_t CommonUtils::DecodeInt16(const char* data) {
    uint16_t result = (static_cast<uint16_t>(data[0]) << 8 & 0xff00)
        | static_cast<uint16_t>(data[1]) & 0x00ff;
    return result;
}

int CommonUtils::GetFileSize(const string& file) {
    struct stat st;
    int ret = stat(file.c_str(), &st);
    if (ret < 0) {
        return -1;
    }
    return st.st_size;
}

std::shared_ptr<CharArray> CommonUtils::ReadFileAll(const string& file) {
    int file_size = GetFileSize(file);
    if (file_size < 0) {
        return nullptr;
    }
    std::shared_ptr<CharArray> result = std::make_shared<CharArray>(file_size);

    int fd = open(file.c_str(), O_RDONLY);
    if (fd <= 0) {
        return nullptr;
    }
    int rt = 0;
    while (true) {
        int bytes = read(fd, result->Data() + rt, file_size - rt);
        if (bytes < 0) {
            if (errno == EINTR) {
                continue;
            }
            else {
                return nullptr;
            }
        }
        else if (bytes == 0) {
            //eof
            break;
        }
        else {
            rt += bytes;
            continue;
        }
    }
    return result;
}