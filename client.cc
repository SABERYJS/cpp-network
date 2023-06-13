
#include "BaseTcpClient.h"
#include<cstdlib>



const char letters[27] = { "abcdefghijklmnopqrstuvwxyz" };

std::unique_ptr<char>  rand_string(int len) {
    std::unique_ptr<char>data(new char[len]);
    for (int i = 0;i < len;i++) {
        data.get()[i] = letters[rand() % 27];
    }
    return data;
}

int main(int argc, char const* argv[]) {
    srand(static_cast<unsigned int>(time(nullptr)));
    BaseTcpClient client("127.0.0.1", 90);
    client.Connect();
    int lc = 100000;
    while (lc >= 0) {
        string data = rand_string(10000).get();
        //std::getline(std::cin, data);
        if (data.size() == 0) {
            continue;
        }
        if (client.Send(data) < 0) {
            exit(1);
        }
        int bytes;
        std::unique_ptr<char>rdata = client.ReadSome(bytes, 256);
        spdlog::info("response: {}", rdata.get());
        --lc;
    }

    return 0;
}
