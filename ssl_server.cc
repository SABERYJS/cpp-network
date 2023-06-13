
#include<spdlog/spdlog.h>
#include"SSLEchoServer.h"
#include"AsyncDispatcher.h"
#include"ThreadsPool.h"
#include"AsyncTimer.h"

int main(int argc, char const* argv[]) {
    spdlog::info("detect cup info,cores is {}", CommonUtils::GetSystemCpuCores());
    ThreadsPool thdPool(CommonUtils::GetSystemCpuCores());
    AsyncDispatcher dispatcher(&thdPool);
    SSLEchoServer server("0.0.0.0", 90, &dispatcher, "/data/cimserver/certs/server.key", "/data/cimserver/certs/server.crt");
    server.Run();
    spdlog::info("server start,wait dispatch");
    thread thd([&dispatcher]() {
        AsyncTimer tm(30, [&dispatcher]() {
            try {
                auto metrics = dispatcher.GetMetrics();
                spdlog::info("dispatcher metrics,active count:{}", metrics.active_count);
            }
            catch (const std::exception& e) {
                spdlog::error("error:{}", e.what());
            }
            });
        sleep(1000);
        });
    thd.detach();
    dispatcher.Run();
    return 0;
}
