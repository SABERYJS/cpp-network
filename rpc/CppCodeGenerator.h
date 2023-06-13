#ifndef MT_CPP_CODE_GENERATOR_H
#define MT_CPP_CODE_GENERATOR_H

#include<iostream>
#include<fstream>
#include"TokensParser.h"
#include<list>
#include<stdexcept>
#include<fmt/format.h>
#include <stdlib.h>

using namespace std;

class CppCodeGenerator {
public:
    CppCodeGenerator(/* args */) {
        int status = system("clang-format --version");
        if (status < 0)
            use_clang_format = false;
        else {
            if (WIFEXITED(status)) {
                use_clang_format = true;
            }
            else {
                use_clang_format = false;
            }
        }
    }
    ~CppCodeGenerator() {}
    void GenerateCppSource(string proto_file_name, string service_name, std::list<rpc_method>& methods);
private:
    bool use_clang_format{false};
    string get_out_file_name(string proto_file, string ext, string mid);
    void create_service_header_file(string file_name, string service_name, std::list<rpc_method>& methods);
    void create_service_source_file(string file_name, string service_name, std::list<rpc_method>& methods);
    void create_service_client_file(string file_name, string service_name, std::list<rpc_method>& methods);
};

#endif