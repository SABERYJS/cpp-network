#ifndef MT_HTTP_REQUEST
#define MT_HTTP_REQUEST

#include<iostream>
#include<string>
#include<map>
#include"HttpHeader.h"
#include"HttpHeaderTypes.h"
#include<variant>
#include<regex>
#include<cstring>
#include<cctype>
#include<spdlog/spdlog.h>
#include <algorithm>
#include "CommonUtils.h"
using namespace std;


static std::regex request_line_pattern{R"(([a-zA-Z]+)\s+([^\s]+)\s+http\/([1-9])\.([0-9]))", std::regex_constants::icase | std::regex_constants::ECMAScript};

static const char* CRCF = "\r\n";
static std::string sget = "get";
static std::string spost = "post";
static std::string sdelete = "delete";
static std::string soptions = "options";
static std::string strace = "trace";
static std::string sconnect = "connect";
static std::string sput = "put";
static std::string shead = "head";

//reference https://developer.mozilla.org/en-US/docs/Web/HTTP/Headers
//reference https://developer.mozilla.org/en-US/docs/Glossary/Request_header
//reference https://datatracker.ietf.org/doc/html/rfc2616#autoid-37
//simple http request parser
//https://datatracker.ietf.org/doc/html/rfc9112
class HttpRequest {
public:
    enum Result {
        Retry,
        Done,
        Failed
    };

    enum request_method {
        Unknown,
        Options,
        Get,
        Head,
        Post,
        Put,
        Delete,
        Trace,
        Connect
    };
private:
    enum parse_step {
        ParseRequestLine,
        ParseRequestHeaders,
        ParseRequestBody,
        ParseRequestDone,
    };

    parse_step step{ parse_step::ParseRequestLine };
    bool error{ false };
    int  counter{ 0 };
    string request_line;
    request_method method{ request_method::Unknown };
    string uri;
    int http_version_main{ 0 };
    int http_version_min{ 0 };
    std::map<string, string>query;
    std::map<string, string>headers;
public:
    HttpRequest(/* args */);
    ~HttpRequest();
    Result Parse(const char* data, size_t len);
    inline int GetCounter()const { return counter; }
    bool CheckHeader(const char* header);
    string GetHeaderValue(const char* header);
    bool CheckDone()const;

    inline request_method GetMethod()const {
        return method;
    }

    inline const string& GetUri()const {
        return uri;
    }

    inline int GetVersionMain()const {
        return http_version_main;
    }
    inline int GetVersionMin()const {
        return http_version_min;
    }
private:
    void parse_request_line(const char* data, size_t len);
    //https://datatracker.ietf.org/doc/html/rfc9112#section-6
    void parse_request_headers(const char* data, size_t len);
    //https://datatracker.ietf.org/doc/html/rfc9112#section-6.1
    void parse_request_body(const char* data, size_t len);
    inline void reset_counter() { counter = 0; }
    inline void incr_counter(int delta) { counter += delta; }
    void report_error(const char* ptr);
    request_method check_request_method(string method);
    void parse_query();
    void parse_query_segment(string s);
    bool find_header(const char* target);
};

#endif