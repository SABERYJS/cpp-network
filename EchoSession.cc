
#include"EchoSession.h"

void EchoSession::ProcessRead(const char* ptr, int size) {
    cout << string(ptr, size) << endl;
    setConsumed(size);
    WriteData(string(ptr, size));
}