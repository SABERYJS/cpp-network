#include "CppCodeGenerator.h"

void CppCodeGenerator::GenerateCppSource(string proto_file_name, string service_name, std::list<rpc_method>& methods) {
    create_service_header_file(get_out_file_name(proto_file_name, "h", ""), service_name, methods);
    create_service_source_file(get_out_file_name(proto_file_name, "cc", ""), service_name, methods);
    create_service_client_file(get_out_file_name(proto_file_name, "h", "Client"), service_name, methods);
}

string  CppCodeGenerator::get_out_file_name(string proto_file, string ext, string mid) {
    int pos = proto_file.find_last_of(".");
    if (pos == string::npos) {
        throw std::runtime_error("invalid proto file name");
    }
    string prefix = proto_file.substr(0, pos);
    if (mid.size() > 0) {
        string out_file = fmt::format("{}{}.{}", prefix, mid, ext);
        return out_file;
    }
    else {
        string out_file = fmt::format("{}.{}", prefix, ext);
        return out_file;
    }
}

void CppCodeGenerator::create_service_header_file(string file_name, string service_name, std::list<rpc_method>& methods) {
    ofstream  out(file_name, std::ios_base::trunc | std::ios_base::out);
    out << "//this file is auto generated by program,do not edit\n";
    out << "#include\"RpcService.h\"\n";
    out << "#include\"RpcMethodHandler.h\"\n";
    out << "#include<functional>\n";
    out << "#include\"protocols.pb.h\"\n";
    out << "using namespace std::placeholders;\n";
    out << "using namespace rpc;\n";
    out << "static const std::string " << service_name << "_method_names[]={";
    int i = 0;
    for (auto it = methods.begin();it != methods.end();it++) {
        if (i == 0) {
            out << "\"" << service_name << "." << it->method_name << "\"";
        }
        else {
            out << ",\"" << service_name << "." << it->method_name << "\"";
        }
        ++i;
    }
    out << "};\n";
    out << "class " << service_name << ":public RpcService {\n";
    out << "public:\n";
    for (auto it = methods.begin();it != methods.end();it++) {
        out << "virtual Status " << it->method_name << "(" << it->request_class << "*request," << it->response_class << "*response)=0;\n";
    }
    out << service_name << "();\n";
    out << "~" << service_name << "();\n";
    out << "};\n";
    out << std::flush;
    if (use_clang_format) {
        system(fmt::format("clang-format -i {} > /dev/null 2>&1", file_name).c_str());
    }
}

void CppCodeGenerator::create_service_source_file(string file_name, string service_name, std::list<rpc_method>& methods) {
    ofstream  out(file_name, std::ios_base::trunc | std::ios_base::out);
    out << "//this file is auto generated by program,do not edit\n";
    out << "#include\"" << service_name << ".h\"\n";
    out << service_name << "::" << service_name << "(){\n";
    int i = 0;
    for (auto it = methods.begin();it != methods.end();it++) {
        out << "RegisterDescriptors(" << service_name << "_method_names[" << i << "],";
        out << "std::make_shared<service_method_descriptor>(" << service_name << "_method_names[" << i << "],";
        out << "new RpcMethodHandler<" << it->request_class << ", " << it->response_class << ">(std::bind(&" << service_name << "::" << it->method_name << ",this,_1,_2))));\n";
        ++i;
    }
    out << "}\n";
    out << service_name << "::~" << service_name << "(){}\n";
    out << std::flush;
    if (use_clang_format) {
        system(fmt::format("clang-format -i {} > /dev/null 2>&1", file_name).c_str());
    }
}

void CppCodeGenerator::create_service_client_file(string file_name, string service_name, std::list<rpc_method>& methods) {
    ofstream  out(file_name, std::ios_base::trunc | std::ios_base::out);
    out << "//this file is auto generated by program,do not edit\n";
    out << "#include\"RpcServiceClient.h\"\n";
    out << "#include<functional>\n";
    out << "#include\"protocols.pb.h\"\n";
    out << "using namespace rpc;\n";
    string client_name = service_name + "Client";
    out << "class " << client_name << ":public RpcServiceClient {\n";
    out << "public:\n";
    out << client_name << "(std::string saddr, short sport):RpcServiceClient(saddr,sport){";
    for (auto it = methods.begin();it != methods.end();it++) {
        out << "addRpcMethod(\"" << service_name << "." << it->method_name << "\");\n";
    }
    out << "Connect();\n";
    out << "}\n";
    out << "~" << client_name << "(){}\n";
    int i = 0;
    for (auto it = methods.begin();it != methods.end();it++) {
        out << "bool " << it->method_name << "(" << it->request_class << "&request," << it->response_class << "&response){\n";
        out << "rpcCall(request,response," << i << ");\n";
        out << "}\n";
        i++;
    }
    out << "};\n";
    out << std::flush;
    if (use_clang_format) {
        system(fmt::format("clang-format -i {} > /dev/null 2>&1", file_name).c_str());
    }
}