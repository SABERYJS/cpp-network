#ifndef MT_DYNAMIC_BYTES_H
#define MT_DYNAMIC_BYTES_H


#include<functional>
#include<cassert>

using namespace std;


#ifndef MT_DYNAMIC_SIZE
#define MT_DYNAMIC_SIZE 256
#endif


//it is not thread safe
class DynamicBytes {
private:
    int buf_size;
    char* ptr;
    int len;
public:
    explicit DynamicBytes(int size);
    DynamicBytes();
    ~DynamicBytes();
    DynamicBytes(const DynamicBytes&& db) = delete;
    DynamicBytes& operator=(const DynamicBytes&& db) = delete;
    void Resize(int size);
    void Prepare(int size);
    void Consume(int size);
    void Commit(int size);
    char* Data();
    char* CData();
    int  Size()const;
    int CSize()const;
    void Clear();
    inline int MaxSize()const { return buf_size; }
private:
    void allocate_new_mem(int size);
    void init_vars();
};



#endif