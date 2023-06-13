#include "HttpRequest.h"

HttpRequest::HttpRequest() {}
HttpRequest::~HttpRequest() {}

HttpRequest::Result HttpRequest::Parse(const char* data, size_t len) {
    if (step == parse_step::ParseRequestDone) {
        return Result::Done;
    }
    else if (error) {
        return Result::Failed;
    }
    reset_counter();
    if (step == parse_step::ParseRequestLine) {
        parse_request_line(data + counter, len);
    }
    if (error) {
        return Result::Failed;
    }
    if (step == parse_step::ParseRequestHeaders && !error) {
        parse_request_headers(data + counter, len);
    }
    if (error) {
        return Result::Failed;
    }
    if (step == parse_step::ParseRequestBody && !error) {
        parse_request_body(data + counter, len);
    }
    if (error) {
        return Result::Failed;
    }
    if (step != parse_step::ParseRequestDone) {
        return Result::Retry;
    }
    return Result::Done;
}

//CRLF=> \r\n
void HttpRequest::parse_request_line(const char* data, size_t len) {
    char* pos = strstr(const_cast<char*>(data), CRCF);
    if (pos == nullptr) {
        return;
    }
    int delta = pos - data;
    request_line = std::move(string(data, delta));
    spdlog::info("request line:{}", request_line);
    std::smatch result;
    //submatch with full match
    spdlog::info("{}", std::regex_match(request_line, result, request_line_pattern));
    if (!std::regex_match(request_line, result, request_line_pattern) || result.size() != 5) {
        report_error("parse request line");
        return;
    }

    method = check_request_method(result[1].str());
    if (method == request_method::Unknown) {
        report_error("unknown request method");
        return;
    }
    spdlog::info("method {}", method);
    uri = result[2];
    spdlog::info("uri {}", uri);
    http_version_main = stoi(result[3]);
    http_version_min = stoi(result[4]);
    spdlog::info("version {}:{}", http_version_main, http_version_min);
    parse_query();
    int qpos = 0;
    if ((qpos = uri.find("?")) != std::string::npos) {
        //strip query string
        uri = uri.substr(0, qpos);
    }
    step = parse_step::ParseRequestHeaders;
    incr_counter(request_line.size() + 2);
}

void HttpRequest::parse_request_headers(const char* data, size_t len) {
    if (step != parse_step::ParseRequestHeaders) {
        return;
    }
    int from = 0;
    while (true) {
        char* pos = strstr(const_cast<char*>(data + from), CRCF);
        if (pos == nullptr) {
            return;
        }
        //find header line
        string header_line(data + from, pos - (data + from));
        spdlog::info("header line: {}", header_line);
        incr_counter(header_line.size() + 2);
        if (header_line.size() == 0) {
            //single crcf,headers done!
            step = parse_step::ParseRequestBody;
            return;
        }

        int epos = header_line.find(":");
        if (epos == std::string::npos) {
            //invalid header
            report_error("invalid request header");
            return;
        }

        //for safety
        //nil:value or key:nil
        if (epos == (header_line.size() - 1) || epos == 0) {
            report_error("invalid request header");
            return;
        }

        string hkey = std::move(header_line.substr(0, epos));
        string hvalue = std::move(header_line.substr(epos + 1));
        CommonUtils::Trim(hkey, " \t");
        CommonUtils::Trim(hvalue, " \t");
        headers[std::move(hkey)] = std::move(hvalue);
        from += 2 + header_line.size();
        if (from >= len) {
            break;
        }
    }
}

bool HttpRequest::find_header(const char* target) {
    if (headers.find(target) != headers.end()) {
        return true;
    }
    else {
        return false;
    }
}

bool HttpRequest::CheckHeader(const char* header) {
    return  find_header(header);
}


string HttpRequest::GetHeaderValue(const char* header) {
    if (CheckHeader(header)) {
        return headers.find(header)->second;
    }
    else {
        return "";
    }
}

bool HttpRequest::CheckDone()const {
    return step == parse_step::ParseRequestDone;
}

//consider how to know thers is a body in the reqeust ?
//check https://greenbytes.de/tech/webdav/rfc7230.html#message.body
//The presence of a message body in a request is signaled by a Content-Length or Transfer-Encoding header field. 
//Request message framing is independent of method semantics, even if the method does not define any use for a message body.
void HttpRequest::parse_request_body(const char* data, size_t len) {
    if (find_header(ContentLengthConst) || find_header(TransferEncodingConst)) {
        report_error("invalid request contain body");
        return;
    }
    step = parse_step::ParseRequestDone;
}

void HttpRequest::report_error(const char* ptr) {
    error = true;
    spdlog::error("http request failed,{}", ptr);
}

HttpRequest::request_method  HttpRequest::check_request_method(string method) {
    CommonUtils::ToLowercase(method);
    if (method == sget) {
        return request_method::Get;
    }
    else if (method == spost) {
        return request_method::Post;
    }
    else if (method == sdelete) {
        return request_method::Delete;
    }
    else if (method == strace) {
        return request_method::Trace;
    }
    else if (method == soptions) {
        return request_method::Options;
    }
    else if (method == sconnect) {
        return request_method::Connect;
    }
    else if (method == shead) {
        return request_method::Head;
    }
    else if (method == sput) {
        return request_method::Put;
    }
    else {
        return request_method::Unknown;
    }
}

void HttpRequest::parse_query() {
    int pos = uri.find("?", 0);
    int last = uri.size() - 1;
    //consider last char
    if (pos != std::string::npos && pos != last) {
        //skip char ?
        int from = pos + 1;
        while (true) {
            int tpos = uri.find_first_of("&", from);
            if (tpos == std::string::npos) {
                parse_query_segment(uri.substr(from, last - from + 1));
                break;
            }
            else {
                parse_query_segment(uri.substr(from, tpos - from));
                if (tpos != last) {
                    from = tpos + 1;
                    continue;
                }
            }
        }
    }
}

void HttpRequest::parse_query_segment(string s) {
    spdlog::info("query segment : {}", s);
    int from = 0;
    int last = s.size() - 1;

    if (s.size() == 1 && s[0] == '=') {
        return;
    }

    int epos = s.find("=");
    if (epos == std::string::npos) {
        query[s.substr(from, last - from + 1)] = "";
    }
    else if (epos == last) {
        //key=nil
        query[s.substr(from, last - from)] = "";
    }
    else if (epos == from) {
        //nil=value
    }
    else {
        query[s.substr(from, epos - from)] = s.substr(epos + 1, last - epos + 1);
    }
}