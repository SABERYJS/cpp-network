
#include "BaseTcpClient.h"
#include<thread>
#include<fmt/format.h>

#include<iostream>

using namespace std;

int main(int argc, char const* argv[]) {
    for (int i = 0;i < 100000;i++) {
        BaseTcpClient client("127.0.0.1", 90);
        client.Connect();
        for (int m = 0;m < 3;m++) {
            string s(fmt::format("send {}\n", m));
            client.Send(s);
        }
    }
    cout << "exit" << endl;
    return 0;
}
