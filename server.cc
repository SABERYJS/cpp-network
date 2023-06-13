
#include<spdlog/spdlog.h>
#include"EchoServer.h"
#include"AsyncDispatcher.h"
#include"ThreadsPool.h"
#include"AsyncTimer.h"

int main(int argc, char const* argv[]) {
    spdlog::info("detect cup info,cores is {}", CommonUtils::GetSystemCpuCores());
    ThreadsPool thdPool(CommonUtils::GetSystemCpuCores());
    AsyncDispatcher dispatcher(&thdPool);
    EchoServer server("0.0.0.0", 90, &dispatcher);
    server.Run();
    spdlog::info("server start,wait dispatch");
    dispatcher.Run();
    return 0;
}
