#include "http/request_parser.hpp"
#include "http/headers.hpp"
#include "http/server.hpp"

#include <cstdlib>

using namespace http;
using namespace net;

/**
 * @brief Constructs a new Request Parser with strict security limits.
 *
 * @param headers_limit Max bytes allowed for the header section.
 * @param body_limit Max bytes allowed for the request body payload.
 * @param request_target_limit Max characters allowed in the URL path.
 */
RequestParser::RequestParser(std::size_t headers_limit, std::size_t body_limit, std::size_t request_target_limit)
    : Parser(headers_limit),
    body_limit_(body_limit),
    request_target_limit_(request_target_limit)
{}

/**
 * @brief Parses the initial HTTP Request-Line.
 *
 * @details Extracts the HTTP Method, the Request Target, and the HTTP Version.
 *          Updates the `req` object in place.
 *
 * @param req The HTTP request object being built.
 * @param raw_buffer The string view containing the very first line of the request.
 * @return A `std::string_view` of the *remaining* unparsed buffer, or a `ServerErr`
 *         if the Request-Line is malformed or exceeds `request_target_limit_`.
 */
std::expected<std::string_view, ServerErr> RequestParser::parse_request_line(Request& req, std::string_view raw_buffer)
{
    std::size_t i = raw_buffer.find(' ');
    req.method = method::from_string(raw_buffer.substr(0, i));
    if (req.method == RequestMethod::None) {
        return std::unexpected(ServerErr(
            StatusCode::NotImplemented,
            "Unknown Request Method -> " + std::string(raw_buffer.substr(0, i))));
    }
    raw_buffer = raw_buffer.substr(i + 1);

    i = raw_buffer.find(' ');
    std::string_view raw_req_target = raw_buffer.substr(0, i);
    if (raw_req_target.size() > request_target_limit_) {
        return std::unexpected(ServerErr(
            StatusCode::UriTooLong,
            "Maximum Uri size is -> " + std::to_string(request_target_limit_)));
    }
    auto req_target = RequestTarget::from(raw_req_target);
    if (!req_target.has_value()) {
        return std::unexpected(ServerErr(
            StatusCode::BadRequest,
            std::move(req_target.error())));
    }
    req.target = *req_target;
    raw_buffer = raw_buffer.substr(i + 1);

    i = raw_buffer.find("\r\n");
    if (raw_buffer.substr(0, i) != kServerHttpVersion) {
        return std::unexpected(ServerErr(
            StatusCode::HttpVersionNotSupported,
            "Server supports -> " + std::string(kServerHttpVersion)));
    }
    raw_buffer = raw_buffer.substr(i + 2);

    return raw_buffer;
}

/**
 * @brief The primary entry point for processing an incoming client connection.
 *
 * @param conn The active network connection.
 * @return A completely parsed and validated `Request` object ready for routing,
 *         or a `ServerErr` on failure.
 */
std::expected<Request, ServerErr> RequestParser::parse_request(Connection& conn)
{
    auto buffer = conn.read_until("\r\n\r\n", headers_limit_);
    if (!buffer.has_value()) {
        if (buffer.error() == StatusCode::ContentTooLarge) {
            return std::unexpected(ServerErr(StatusCode::RequestHeaderFieldsTooLarge));
        }
        return std::unexpected(ServerErr(buffer.error()));
    }

    // Add for easier headers parsing
    *buffer += "\r\n";

    Request req;

    auto buf = parse_request_line(req, *buffer);
    if (!buf.has_value()) {
        return std::unexpected(buf.error());
    }

    auto headers = parse_headers(*buf);
    if (!headers.has_value()) {
        return std::unexpected(headers.error());
    }

    for (const auto& [key, value] : *headers) {
        if (key == header::kHost) {
            req.host = true;
        } else if (key == header::Connection::name) {
            if (value == header::Connection::kKeepAlive) {
                req.keep_alive = true;
            } else if (value == header::Connection::kClose) {
                req.close = true;
            }
        } else if (key == header::kContentLength) {
            try {
                req.content_length = std::stoi(value);
            } catch (const std::exception&) {
                return std::unexpected(ServerErr(StatusCode::BadRequest, "Couldn't parse Content-Length"));
            }
        }
    }

    if (!req.host) {
        return std::unexpected(ServerErr(StatusCode::BadRequest, "Missing Host Header"));
    }

    if (req.content_length > 0) {
        if (req.content_length <= body_limit_) {
            auto body = conn.read(req.content_length);
            if (!body.has_value()) {
                return std::unexpected(body.error());
            }
            req.body = std::move(*body);
        } else {
            return std::unexpected(ServerErr(StatusCode::ContentTooLarge));
        }
    }

    return req;
}
