
#include<spdlog/spdlog.h>
#include"RpcServer.h"
#include"AsyncDispatcher.h"
#include"ThreadsPool.h"
#include"AsyncTimer.h"
#include "NormalServiceImpl.h"
#include<cxxopts.hpp>
#include"CommonUtils.h"
#include"AppConfig.h"
#include<spdlog/sinks/rotating_file_sink.h>

int main(int argc, char const* argv[]) {
    //spdlog::info("detect cup info,cores is {}", CommonUtils::GetSystemCpuCores());

    cxxopts::Options options("rpcserver", "A brief description");

    options.add_options()
        ("c,config", "config file", cxxopts::value<std::string>()->default_value("/etc/rpcserver/config.json"))
        ("h,help", "print usage");

    auto result = options.parse(argc, argv);

    if (result.count("help")) {
        std::cout << options.help() << std::endl;
        exit(0);
    }

    AppConfig::Unmarshal(result["config"].as<std::string>());
    if (g_config.daemon) {
#ifndef WITH_SYSTEMD_ENABLED
        CommonUtils::Daemon(g_config.workdir);
        std::shared_ptr<spdlog::logger> logger = spdlog::rotating_logger_mt("rpcserverlog",
            g_config.workdir + "/logs/" + g_config.log_file, g_config.log_file_max_size, g_config.log_max_files);
        spdlog::set_default_logger(logger);
#endif
    }

    ThreadsPool thdPool(CommonUtils::GetSystemCpuCores());
    AsyncDispatcher dispatcher(&thdPool);
    RpcServer server(g_config.server_address, g_config.server_port, &dispatcher);
    std::shared_ptr<RpcService>normalSvc(new NormalServiceImpl);
    server.RegisterService(normalSvc);
    server.Run();
    spdlog::info("server start,wait dispatch");
    dispatcher.Run();
    return 0;
}
