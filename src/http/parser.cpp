#include "http/parser.hpp"

#include <algorithm>

using namespace http;

Parser::Parser(std::size_t headers_limit)
    : headers_limit_(headers_limit)
{}

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

