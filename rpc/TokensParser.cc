#include "TokensParser.h"


void TokensParser::parse_proto_file(string file_path) {
    std::ifstream file(file_path);
    if (!file) {
        throw std::runtime_error("proto file not exist");
    }
    std::string fc((std::istreambuf_iterator<char>(file)),
        (std::istreambuf_iterator<char>()));
    file.close();
    parse_tokens(fc);
}

void TokensParser::parse_tokens(string fc) {
    int end = parse_service_name(fc);
    if (end >= (fc.size() - 1)) {
        throw std::runtime_error("parse invalid");
    }
    fc = fc.substr(end + 1);
    while (true) {
        if (parse_last(fc)) {
            //parse done
            return;
        }
        end = parse_rpc_methods(fc);
        if (end >= (fc.size() - 1)) {
            throw std::runtime_error("parse invalid");
        }
        fc = fc.substr(end + 1);
    }
}

bool TokensParser::is_space(const char c) {
    if (c == space[0] || c == tab[0] || c == newline[0] || c == back[0]) {
        return true;
    }
    else {
        return false;
    }
}

int TokensParser::parse_service_name(string fc) {
    int pos = fc.find_first_of(alphabets);
    if (pos == string::npos) {
        throw std::runtime_error("parse failed");
    }
    for (int i = 0;i < pos;i++) {
        if (!is_space(fc[i])) {
            throw std::runtime_error("parse failed");
        }
    }
    int lbpos = fc.find_first_of(lbracket);
    if (lbpos == string::npos) {
        throw std::runtime_error("parse failed");
    }
    std::smatch match;
    string search = fc.substr(pos, lbpos - pos + 1);
    if (!std::regex_match(search, match, service_pattern)) {
        throw std::runtime_error("parse failed");
    }
    //parse rpc methods
    service_name = match[1];
    return lbpos;
}

int TokensParser::parse_rpc_methods(string fc) {
    int pos = fc.find_first_of(alphabets);
    if (pos == string::npos) {
        throw std::runtime_error("parse failed");
    }
    for (int i = 0;i < pos;i++) {
        if (!is_space(fc[i])) {
            throw std::runtime_error("parse failed");
        }
    }

    int lbpos = fc.find_first_of(semicolon);
    if (lbpos == string::npos) {
        throw std::runtime_error("parse failed");
    }
    std::smatch match;
    string search = fc.substr(pos, lbpos - pos + 1);
    if (!std::regex_match(search, match, rpc_method_pattern)) {
        throw std::runtime_error("parse failed");
    }
    rpc_method rm{ .method_name = match[1],.request_class = match[2],.response_class = match[3] };
    methods.push_back(rm);
    return lbpos;
}

bool TokensParser::parse_last(string fc) {
    std::smatch match;
    if (!std::regex_match(fc, match, last_match_pattern)) {
        return false;
    }
    else {
        return true;
    }
}