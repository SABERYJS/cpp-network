#ifndef MT_APP_CONFIG_H
#define MT_APP_CONFIG_H


#include<string>
#include<nlohmann/json.hpp>
#include<fstream>
#include<stdexcept>
#include<fmt/format.h>
#include"CommonUtils.h"

using namespace std;
using json = nlohmann::json;



#define SIZEM 1024*1024

class AppConfig;

//for nlohmann json
void to_json(json& j, const AppConfig& p);

void from_json(const json& j, AppConfig& p);

class AppConfig {
public:
    string server_address{};
    short server_port{};
    bool daemon{ false };
    string workdir{};
    string log_file{};
    int log_file_max_size{ SIZEM * 5 };//5M
    int log_max_files{ 3 };
public:
    AppConfig(/* args */) {}
    ~AppConfig() {}
    static  void Unmarshal(string file);
};

extern AppConfig  g_config;

#endif