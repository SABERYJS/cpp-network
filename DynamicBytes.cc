#include "DynamicBytes.h"


DynamicBytes::DynamicBytes(int size) {
    init_vars();
    allocate_new_mem(size);
}

DynamicBytes::DynamicBytes() {
    init_vars();
    allocate_new_mem(MT_DYNAMIC_SIZE);
}

DynamicBytes::~DynamicBytes() {
    delete[] ptr;
}

void DynamicBytes::Resize(int size) {
    allocate_new_mem(size);
}

void DynamicBytes::Prepare(int size) {
    if ((buf_size - len) < size) {
        allocate_new_mem(buf_size + (size - (buf_size - len)));
    }
}

void DynamicBytes::Consume(int size) {
    if (size <= 0 || size > len) {
        return;
    }
    //copy(ptr, ptr + size, ptr);
    copy(ptr + size, ptr + len, ptr);
    len -= size;
}

void DynamicBytes::Commit(int size) {
    len += size;
}

void DynamicBytes::allocate_new_mem(int size) {
    char* new_ptr = new char[size];
    assert(new_ptr != nullptr);
    if (ptr != nullptr) {
        copy(ptr, ptr + len, new_ptr);
        delete[] ptr;
    }
    ptr = new_ptr;
    buf_size = size;
}

void DynamicBytes::init_vars() {
    len = 0;
    ptr = nullptr;
    buf_size = 0;
}

char* DynamicBytes::Data() {
    return static_cast<char*>(ptr);
}

char* DynamicBytes::CData() {
    return static_cast<char*>(ptr + len);
}

int  DynamicBytes::Size()const {
    return len;
}

int DynamicBytes::CSize()const {
    return buf_size - len;
}

void DynamicBytes::Clear() {
    len = 0;
}