#include "http/request_parser.hpp"
#include "http/server.hpp"

#include <stdexcept>
#include <errno.h>
#include <iostream>
#include <cctype>
#include <cstdlib>

using namespace http;
using namespace net;

std::expected<RequestHeader, StatusCode> RequestParser::parse_header(std::string_view line) noexcept
{
    std::size_t i = line.find(':');
    if (i == std::string_view::npos) {
        return std::unexpected(StatusCode::BadRequest);
    }

    std::string_view key = line.substr(0, i);
    std::string_view value = line.substr(i + 1);

    while (!value.empty() && std::isspace(value.front())) {
        value.remove_prefix(1);
    }
    while (!value.empty() && std::isspace(value.back())) {
        value.remove_suffix(1);
    }

    return RequestHeader{std::string(key), std::string(value)};
}

std::expected<Request, StatusCode> RequestParser::parse_headers(std::string_view headers) noexcept
{
    Request req;

    std::size_t i = headers.find(' ');
    req.method = to_request_method(headers.substr(0, i));
    if (req.method == RequestMethod::None) {
        return std::unexpected(StatusCode::NotImplemented);
    }
    headers = headers.substr(i + 1);

    i = headers.find(' ');
    // TODO: Accept different request target forms than relative path
    // also accept query string in relative path
    req.relative_path = headers.substr(0, i);
    if (req.relative_path.size() > request_target_limit_) {
        return std::unexpected(StatusCode::UriTooLong);
    }
    headers = headers.substr(i + 1);

    i = headers.find("\r\n");
    if (headers.substr(0, i) != kServerHttpVersion) {
        return std::unexpected(StatusCode::HttpVersionNotSupported);
    }
    headers = headers.substr(i + 1);

    headers = headers.substr(headers.find("\r\n") + 2);

    std::string_view line;

    while ((i = headers.find("\r\n")) != std::string_view::npos)  {
        auto header = parse_header(headers.substr(0, i));

        if (!header.has_value()) {
            return std::unexpected(header.error());
        }

        if (header->key == "Host") {
            req.host = true;
        } else if (header->key == "Connection") {
            if (header->value == "keep-alive") {
                req.keep_alive = true;
            } else if (header->value == "close") {
                req.close = true;
            }
        } else if (header->key == "Content-Length") {
            // TODO: What if it is 0 or negative?
            req.content_length = std::stoi(header->value);
        }

        headers = headers.substr(i + 2);
    }

    return req;
}

RequestParser::RequestParser(std::size_t headers_limit, std::size_t body_limit, std::size_t request_target_limit)
    : headers_limit_(headers_limit),
    body_limit_(body_limit),
    request_target_limit_(request_target_limit)
{}

std::expected<Request, http::StatusCode> RequestParser::parse_request(Connection& conn) noexcept
{
    Connection::ReadResult res = conn.read_until("\r\n\r\n", headers_limit_);
    if (res.status == StatusCode::ContentTooLarge) {
        return std::unexpected(StatusCode::RequestHeaderFieldsTooLarge);
    } else if (res.status != StatusCode::Ok) {
        return std::unexpected(res.status);
    }

    auto req = parse_headers(res.data);
    if (!req.has_value()) {
        return std::unexpected(req.error());
    }

    res = conn.read_until("", body_limit_);
    if (res.status != StatusCode::Ok) {
        return std::unexpected(res.status);
    }

    req->body = std::move(res.data);
    return req;
}
