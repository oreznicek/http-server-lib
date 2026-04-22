#ifndef _TEST_RESPONSE_PARSER_HPP
#define _TEST_RESPONSE_PARSER_HPP

#include <http/parser.hpp>
#include <http/connection.hpp>
#include <net/socket.hpp>

namespace http {

class ResponseParser : Parser {
    std::expected<std::string_view, std::string> parse_status_line(Response& req, std::string_view raw_buffer);
public:
    ResponseParser();
    std::expected<Response, std::string> parse_response(Connection& conn);
};

} // end of `http` namespace

#endif
