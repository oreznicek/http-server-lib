#ifndef _HTTP_REQUEST_PARSER_HPP
#define _HTTP_REQUEST_PARSER_HPP

#include "http/http.hpp"
#include "net/socket.hpp"

#include <string_view>

namespace http {

struct RequestHeader {
    std::string key;
    std::string value;
};

class RequestParser {
    static constexpr int CHUNK = 1024;
    static constexpr std::string HEADERS_END = "\r\n\r\n";
    std::size_t headers_limit;
    std::size_t body_limit;

    std::string read_headers(net::ClientSocket** socket);
    RequestHeader parse_header(std::string_view line);
    Request parse_headers(std::string_view headers);
    std::string read_body(net::ClientSocket** socket);
public:
    RequestParser(std::size_t headers_limit, std::size_t body_limit);
    Request parse_request(net::ClientSocket* socket);
};

void send_error_response(net::ClientSocket& csock, StatusCode code);
void send_error_response(net::ClientSocket& csock, StatusCode code, std::string&& message);

} // end of `http` namespace

#endif
