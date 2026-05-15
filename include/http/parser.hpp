#ifndef _HTTP_PARSER_HPP
#define _HTTP_PARSER_HPP

#include "http/headers.hpp"
#include "http/response.hpp"

#include <expected>
#include <string_view>

namespace http {

/**
 * @brief Base class for HTTP parsing.
 *
 * @details Provides common functionality required for parsing both HTTP requests
 *          and responses, specifically focusing on safely extracting and validating
 *          HTTP headers.
 */
class Parser {
protected:
    /// @brief Maximum allowed total size for the header block.
    std::size_t headers_limit_;
    Parser(std::size_t headers_limit);
    std::expected<Header, ServerErr> parse_header(std::string_view line);
    std::expected<Headers, ServerErr> parse_headers(std::string_view buffer);
};

} // end of `http` namespace

#endif
