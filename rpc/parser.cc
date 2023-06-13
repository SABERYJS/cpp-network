
#include "CppCodeGenerator.h"
#include "TokensParser.h"
#include<string>

using namespace std;
int main(int argc, char const* argv[]) {
    TokensParser parser;
    string proto_file = "/data/cimserver/rpc/NormalService.proto";
    parser.parse_proto_file(proto_file);
    CppCodeGenerator generator;
    generator.GenerateCppSource(proto_file, parser.GetServiceName(), parser.GetMethods());
    return 0;
}
