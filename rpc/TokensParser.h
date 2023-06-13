#ifndef MT_TOKENS_PARSER_H
#define MT_TOKENS_PARSER_H


#include<regex>
#include<string>
#include<fstream>
#include<stdexcept>
#include<list>


using namespace std;


//proto characters
static const string lbracket = "{";
static const string rbracket = "}";
static const string semicolon = ";";
static const string lparentheses = "(";
static const string rparentheses = ")";
static const string space = " ";
static const string tab = "\t";
static const string newline = "\n";
static const string back = "\r";
static const string service_tag = "service";
static const string rpc_tag = "rpc";
static const string returns_tag = "returns";

static const string alphabets{"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz"};

//rpc pattern
static const  std::regex rpc_method_pattern{R"(rpc\s+([a-zA-Z]+)\s*\(\s*([a-zA-Z]+)\s*\)\s*returns\s*\(\s*([a-zA-Z]+)\s*\)\s*\{\s*\}\s*;)", std::regex_constants::icase | std::regex_constants::ECMAScript};
//service pattern
static const std::regex service_pattern{R"(service\s+([a-zA-Z]+)\s*\{)", std::regex_constants::icase | std::regex_constants::ECMAScript};
//match last
static const std::regex last_match_pattern{R"([\s\t\n]*\}[\s\t\n]*)", std::regex_constants::icase | std::regex_constants::ECMAScript};

struct rpc_method {
    string method_name;
    string request_class;
    string response_class;
};



class TokensParser {
private:
    enum identity_type {
        identity_lbracket,
        identity_rbracket,
        identity_semicolon,
        identity_lparentheses,
        identity_rparentheses,
        identity_space,
        identity_tab,
        identity_newline,
        identity_back,
        identity_name //identity name
    };


    enum expect_type {
        expect_service_tag,
        expect_service_name,
        expect_lbracket,
        expect_rbracket,
        expect_rpc_tag,
        expect_space,
        expect_lparentheses,
        expect_rparentheses,
        expect_returns_tag,
        expect_semicolon
    };

    /* data */
    struct identity {
        identity_type itype;
        string value;
    };

    string service_name;

    std::list<rpc_method>methods;

public:
    TokensParser(/* args */) {}
    ~TokensParser() {}
    void parse_proto_file(string file_path);
    std::list<rpc_method>& GetMethods() { return methods; }
    std::string& GetServiceName() { return service_name; }
private:
    void parse_tokens(string fc);
    bool is_space(const char c);
    int parse_service_name(string fc);
    int parse_rpc_methods(string fc);
    bool parse_last(string fc);
};


#endif