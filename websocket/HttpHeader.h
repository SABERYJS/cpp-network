#ifndef MT_HTTP_HEADER_H
#define MT_HTTP_HEADER_H


#include"HttpHeaderTypes.h"
#include<string>

using namespace std;

template<class Value>
class HttpHeader {
private:
    string key;
    Value value;
public:
    const string& getKey()const {
        return key;
    }

    const Value& getValue()const {
        return value;
    }

    void SetKey(string&& key) {
        this->key = key;
    }

    void SetKey(string& key) {
        this->key = key;

    }

    void SetKey(const char* ptr, size_t len) {
        this->key = string(ptr, len);
    }

    void SetValue(Value& value) {
        this->value = value;
    }

    void SetValue(Value&& value) {
        this->value = value;
    }

    inline HttpHeaderTypes type() const {
        return HttpHeaderTypes::Unknown;
    }
};
#endif