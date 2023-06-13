
#include<spdlog/spdlog.h>
#include"WebsocketServer.h"
#include"AsyncDispatcher.h"
#include"ThreadsPool.h"

int main(int argc, char const* argv[]) {
    spdlog::info("detect cup info,cores is {}", CommonUtils::GetSystemCpuCores());
    ThreadsPool thdPool(CommonUtils::GetSystemCpuCores());
    AsyncDispatcher dispatcher(&thdPool);
    WebsocketServer server("0.0.0.0", 10000, &dispatcher);
    server.Run();
    spdlog::info("server start,wait dispatch");
    dispatcher.Run();
    return 0;
}
