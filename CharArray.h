#ifndef MT_CHAR_ARRAY_H
#define MT_CHAR_ARRAY_H

#include<functional>
#include<algorithm>
#include<spdlog/spdlog.h>


using namespace std;

class CharArray {
private:
    char* data;
    int size;
public:
    CharArray() = delete;

    CharArray(int len) :size(len) {
        data = new char[size];
        spdlog::info("default");
    }

    CharArray(CharArray& s) {
        this->data = new char[s.size];
        copy(s.begin(), s.end(), this->data);
        this->size = s.size;
        spdlog::info("copy");
    }

    CharArray(CharArray&& s) {
        this->data = s.data;
        s.data = nullptr;
        s.size = 0;
        copy(s.begin(), s.end(), this->data);
        this->size = s.size;
        spdlog::info("move");
    }

    void operator=(CharArray& s) {
        delete[] data;
        this->data = new char[s.size];
        copy(s.begin(), s.end(), this->data);
        this->size = s.size;
        spdlog::info("assign");
    }

    ~CharArray() {
        if (data != nullptr) {
            delete[] data;
        }
    }

    inline int Size() const { return size; }

    inline char* Data() { return data; }

    inline char* begin() { return data; }

    inline char* end() { return data + size; }
};

#endif