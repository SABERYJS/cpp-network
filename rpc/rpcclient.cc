#include"NormalServiceClient.h"
#include "protocompiled/protocols.pb.h"
#include<ctime>
#include<spdlog/spdlog.h>
#include<thread>
#include<chrono>

using namespace std;

int main(int argc, char const* argv[]) {
    NormalServiceClient client("127.0.0.1", 90);
    for (int i = 0;i < 1000;i++) {
        rpc::GetTimeReq treq;
        treq.set_ctime(time(nullptr));
        rpc::GetTimeResp tresp;
        if (client.GetTime(treq, tresp)) {
            spdlog::info("get time resp,{}", tresp.time());
        }
        else {
            spdlog::error("getTime failed");
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    return 0;
}
