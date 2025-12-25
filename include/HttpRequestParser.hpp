#ifndef _HTTP_REQUEST_PARSER_HPP
#define _HTTP_REQUEST_PARSER_HPP

#include "http.hpp"
#include "HttpServer.hpp"

#include <map>

namespace http {

struct RequestHeader {
    std::string key;
    std::string value;
};

class RequestParser {
    static constexpr std::size_t CHUNK = 1024;
    static constexpr std::string HEADERS_END = "\r\n\r\n";
    std::size_t headers_limit;
    std::size_t body_limit;

    std::string read_headers(ClientSocket** socket);
    HttpRequest parse_headers(const std::string& headers);
    std::string read_body(ClientSocket** socket);
public:
    HttpRequestParser(std::size_t headers_limit, std::size_t body_limit);
    HttpRequest parse_request(ClientSocket* socket);
};

}

#endif
