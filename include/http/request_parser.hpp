#ifndef _HTTP_REQUEST_PARSER_HPP
#define _HTTP_REQUEST_PARSER_HPP

#include "http/http.hpp"
#include "http/connection.hpp"
#include "net/socket.hpp"

#include <string>
#include <string_view>
#include <expected>

namespace http {

struct RequestHeader {
    std::string key;
    std::string value;
};

class RequestParser {
    std::size_t headers_limit_;
    std::size_t body_limit_;
    std::size_t request_target_limit_;

    std::expected<RequestHeader, StatusCode> parse_header(std::string_view line) noexcept;
    std::expected<Request, StatusCode> parse_headers(std::string_view headers) noexcept;
public:
    RequestParser(std::size_t headers_limit, std::size_t body_limit, std::size_t request_target_limit);
    std::expected<Request, StatusCode> parse_request(Connection& conn) noexcept;
};

} // end of `http` namespace

#endif
