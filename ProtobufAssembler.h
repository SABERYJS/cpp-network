#ifndef MT_PROTOBUF_ASSEMBLER_H
#define MT_PROTOBUF_ASSEMBLER_H

#include<type_traits>
#include<google/protobuf/message.h>
#include<cassert>
#include"DynamicBytes.h"
#include "CommonUtils.h"

using namespace std;


//prefix 2 bytes+payload(Message object length)
template<class T>
class ProtobufAssembler {
public:
    ProtobufAssembler(/* args */) {
        static_assert(std::is_base_of<google::protobuf::Message, T>::value, "T must derived from google::protobuf::Message");
    }
    ~ProtobufAssembler() {}

    int Encode(T& msg, DynamicBytes& dest) {
        int size = msg.ByteSize() + 2;
        dest.Prepare(size);
        CommonUtils::EncodeInt16(dest.CData(), msg.ByteSize());
        if (!msg.SerializeToArray(dest.CData() + 2, dest.CSize() - 2)) {
            return -1;
        }
        dest.Commit(size);
        return 0;
    }

    int Decode(DynamicBytes& src, T& dest) {
        if (src.Size() < 2) {
            return 0;
        }

        int size = CommonUtils::DecodeInt16(src.Data());
        if (size <= 0) {
            return -1;
        }
        if ((src.Size() - 2) < size) {
            return 0;
        }

        if (!dest.ParseFromArray(src.Data() + 2, size)) {
            return -1;
        }

        src.Consume(2 + dest.ByteSize());
        return size;
    }

    int Decode(const char* data, size_t len, T& dest) {
        if (len < 2) {
            return 0;
        }
        int size = CommonUtils::DecodeInt16(data);
        if (size <= 0) {
            return -1;
        }
        if ((len - 2) < size) {
            return 0;
        }
        if (!dest.ParseFromArray(data + 2, size)) {
            return -1;
        }

        return 2 + size;
    }
};


#endif