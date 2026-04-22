#ifndef _HTTP_REQUEST_PARSER_HPP
#define _HTTP_REQUEST_PARSER_HPP

#include "http/http.hpp"
#include "http/parser.hpp"
#include "http/connection.hpp"

namespace http {

class RequestParser : Parser {
    std::size_t body_limit_;
    std::size_t request_target_limit_;
    std::expected<std::string_view, ServerErr> parse_request_line(Request& req, std::string_view raw_buffer);
public:
    RequestParser(std::size_t headers_limit, std::size_t body_limit, std::size_t request_target_limit);
    std::expected<Request, ServerErr> parse_request(Connection& conn);
};

} // end of `http` namespace

#endif
