#include "http/parser.hpp"

#include <algorithm>

using namespace http;

/**
 * @brief Initializes the base parser with specific security constraints.
 * @param headers_limit The maximum byte size allowed for the HTTP headers.
 */
Parser::Parser(std::size_t headers_limit)
    : headers_limit_(headers_limit)
{}

/**
 * @brief Parses a single raw HTTP header line.
 *
 * @details Splits a string like "Content-Type: text/html" into a key-value pair,
 *          automatically converting the key to lowercase for case-insensitive lookups.
 *
 * @param line A single line of text from the HTTP header block (excluding CRLF).
 * @return A parsed `Header` key-value pair, or a `ServerErr` if the syntax is invalid.
 */
std::expected<Header, ServerErr> Parser::parse_header(std::string_view line)
{
    std::size_t i = line.find(':');
    if (i == std::string_view::npos) {
        return std::unexpected(ServerErr(StatusCode::BadRequest, "Missing Header Colon"));
    }

    std::string_view raw_key = line.substr(0, i);
    if (raw_key.empty()) {
        return std::unexpected(ServerErr(StatusCode::BadRequest, "Missing Header Key"));
    }

    std::string_view value = line.substr(i + 1);

    while (!value.empty() && std::isspace(value.front())) {
        value.remove_prefix(1);
    }
    while (!value.empty() && std::isspace(value.back())) {
        value.remove_suffix(1);
    }

    std::string key(raw_key);

    std::transform(key.begin(), key.end(), key.begin(),
        [](unsigned char c) {
            return std::tolower(c);
        });

    return std::make_pair(std::string(key), std::string(value));
}

/**
 * @brief Parses an entire block of HTTP headers.
 *
 * @details Iterates through the raw header string, splitting by CRLF (\r\n),
 *          and populating a map of headers. Enforces the `headers_limit_`
 *          to ensure the buffer doesn't exceed safe boundaries.
 *
 * @param buffer The complete raw string view of the header section.
 * @return A fully populated `Headers` map, or a `ServerErr` on failure.
 */
std::expected<Headers, ServerErr> Parser::parse_headers(std::string_view buffer)
{
    Headers headers;

    std::size_t i;
    while ((i = buffer.find("\r\n")) != std::string_view::npos)  {
        auto header = parse_header(buffer.substr(0, i));
        if (!header.has_value()) {
            return std::unexpected(header.error());
        }
        headers.insert(*header);
        buffer = buffer.substr(i + 2);
    }

    return headers;
}

