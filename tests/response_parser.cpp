#include "response_parser.hpp"
#include "http/server.hpp"

#include <string>
#include <string_view>
#include <iostream>

using namespace http;

ResponseParser::ResponseParser()
    : Parser(8 * 1024)
{}

std::expected<std::string_view, std::string> ResponseParser::parse_status_line(Response& res, std::string_view raw_buffer)
{
    std::size_t i = raw_buffer.find(' ');
    if (raw_buffer.substr(0, i) != kServerHttpVersion) {
        return to_string(StatusCode::HttpVersionNotSupported);
    }
    raw_buffer = raw_buffer.substr(i + 1);

    i = raw_buffer.find(' ');
    try {
        int code = std::stoi(std::string(raw_buffer.substr(0, i)));
        res.code = static_cast<StatusCode>(code);
    } catch (...) {
        return "Converting " + std::string(raw_buffer.substr(0, i)) + " to status code failed";
    }
    raw_buffer = raw_buffer.substr(i + 1);

    i = raw_buffer.find("\r\n");
    return raw_buffer.substr(i + 2);
}

std::expected<Response, std::string> ResponseParser::parse_response(Connection& conn)
{
    auto buffer = conn.read_until("\r\n\r\n", headers_limit_);
    if (!buffer.has_value()) {
        return std::unexpected("Error [read response headers]: " + std::to_string((int)buffer.error()));
    }

    // Add for easier headers parsing
    *buffer += "\r\n";

    Response res;
    auto buf = parse_status_line(res, *buffer);
    if (!buf.has_value()) {
        return std::unexpected("Error [parse status line]: " + buf.error());
    }

    auto headers = parse_headers(*buf);
    if (!headers.has_value()) {
        return std::unexpected("Error [parse headers]: " + headers.error().message);
    }

    res.headers = *headers;

    if (auto it = headers->find("Content-Length"); it != headers->end()) {
        int content_length;
        try {
            content_length = std::stoi(it->second);
        } catch (...) {
            return std::unexpected("Converting Content-Length: " + it->second + " int failed");
        }
        auto body = conn.read(content_length);
        if (!body.has_value()) {
            return std::unexpected("Error [read body]: " + std::to_string((int)body.error()));
        }
        res.body = std::move(*body);
    }

    return res;
}
