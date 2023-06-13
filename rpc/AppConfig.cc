#include "AppConfig.h"

void to_json(json& j, const AppConfig& p) {

}

void from_json(const json& j, AppConfig& p) {
    j.at("server_address").get_to(p.server_address);
    j.at("server_port").get_to(p.server_port);
    j.at("daemon").get_to(p.daemon);
    j.at("workdir").get_to(p.workdir);
    j.at("log_file").get_to(p.log_file);
    j.at("log_file_max_size").get_to(p.log_file_max_size);
    j.at("log_max_files").get_to(p.log_max_files);
}

void AppConfig::Unmarshal(string file) {
    std::shared_ptr<CharArray>file_data = CommonUtils::ReadFileAll(file);
    if (!file_data) {
        throw std::runtime_error(fmt::format("read file {} failed", file));
    }
    try {
        json jdata = json::parse(file_data->Data());
        from_json(jdata, g_config);
    }
    catch (json::parse_error& error) {
        throw std::runtime_error(fmt::format("parse json file {} failed", file, error.what()));
    }
}

AppConfig  g_config;