#ifndef _HTTP_PARSER_HPP
#define _HTTP_PARSER_HPP

#include "http/http.hpp"

#include <expected>
#include <string_view>

namespace http {

class Parser {
protected:
    std::size_t headers_limit_;
    Parser(std::size_t headers_limit);
    std::expected<Header, ServerErr> parse_header(std::string_view line);
    std::expected<Headers, ServerErr> parse_headers(std::string_view buffer);
};

} // end of `http` namespace

#endif
