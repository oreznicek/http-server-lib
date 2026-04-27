#include "http/http.hpp"
#include "http/server.hpp"

#include <format>
#include <iostream>

using namespace http;

ServerErr::ServerErr(StatusCode code)
    : code(code)
{}

ServerErr::ServerErr(StatusCode code, std::string&& msg)
    : code(code), message(msg)
{}

RequestMethod http::to_request_method(std::string_view str)
{
    if (str == "GET") return RequestMethod::Get;
    else if (str == "POST") return RequestMethod::Post;
    else if (str == "PUT") return RequestMethod::Put;
    else if (str == "DELETE") return RequestMethod::Delete;
    return RequestMethod::None;
}

std::string http::to_string(StatusCode code)
{
    switch (code) {
        case StatusCode::Ok:
            return "OK";
        case StatusCode::BadRequest:
            return "Bad Request";
        case StatusCode::NotFound:
            return "Not Found";
        case StatusCode::RequestTimeout:
            return "Request Timeout";
        case StatusCode::ContentTooLarge:
            return "Content Too Large";
        case StatusCode::UriTooLong:
            return "URI Too Long";
        case StatusCode::RequestHeaderFieldsTooLarge:
            return "Request Header Field Too Large";
        case StatusCode::InternalServerError:
            return "Internal Server Error";
        case StatusCode::NotImplemented:
            return "Not Implemented";
        case StatusCode::HttpVersionNotSupported:
            return "HTTP Version Not Supported";
        case StatusCode::None:
            return "";
    }
    return "";
}

static std::string normalize_path(const std::string& path)
{
    std::vector<std::string> parts;
    std::string token;
    std::istringstream token_stream(path);

    while (std::getline(token_stream, token, '/')) {
        if (token == "" || token == ".") {
            continue;
        } else if (token == "..") {
            if (!parts.empty()) {
                parts.pop_back();
            }
        } else {
            parts.push_back(token);
        }
    }

    std::string normalized;
    if (!parts.empty()) {
        normalized += parts.front();
    }
    for (std::size_t i = 1; i < parts.size(); ++i) {
        normalized += "/" + parts[i];
    }

    return normalized;
}

RequestTarget::RequestTarget()
    : relative_path("")
{}

RequestTarget::RequestTarget(std::string&& relative_path)
    : relative_path(normalize_path(relative_path))
{}

std::expected<RequestTarget, std::string> RequestTarget::from(std::string_view raw_target)
{
    static constexpr std::string_view kScheme = "http:";
    static constexpr std::string_view kAuthorityDelimiter = "//";

    if (raw_target.empty()) {
        return std::unexpected("Missing Request Target");
    }

    std::string_view buf = raw_target;
    bool is_absolute_form = buf.starts_with(kScheme);

    if (is_absolute_form) {
        buf = buf.substr(kScheme.size());
        if (buf.starts_with(kAuthorityDelimiter)) {
            buf = buf.substr(kAuthorityDelimiter.size());
        } else {
            return std::unexpected("Missing " + std::string(kAuthorityDelimiter) + " in absolute-form request target: " + std::string(raw_target));
        }
    }

    std::size_t i = buf.find('/');
    if (i == 0 && is_absolute_form) {
        return std::unexpected("Missing authority part in request target: " + std::string(raw_target));
    } else if (i == std::string_view::npos) {
        return RequestTarget();
    } else if (is_absolute_form) {
        buf = buf.substr(i + 1);
    }

    return RequestTarget(std::string(buf));
}

constexpr std::string_view kDefaultErrorTemplate = R"html(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <title>Error {0}</title>
    <style>
        body {{
            font-family: sans-serif;
            display: flex;
            justify-content: center;
            align-items: center;
            height: 100vh;
            margin: 0;
            background-color: #f9f9f9;
            color: #333;
            text-align: center;
        }}
        h1 {{ margin: 0; font-size: 8rem; color: #e74c3c; }}
        p {{ margin: 0; font-size: 3rem; color: #666; }}
    </style>
</head>
<body>
    <div>
        <h1>{0}</h1>
        <p>{1}</p>
    </div>
</body>
</html>
)html";

std::string Response::error_template_(kDefaultErrorTemplate);

Response::Response()
    : code(StatusCode::None), body("")
{}

Response::Response(StatusCode code)
    : code(code), body("")
{}

Response::Response(ServerErr err)
    : Response(err.code)
{
    int code = (int)err.code;
    std::string message = err.message.empty() ? ::to_string(err.code) : std::move(err.message);
    add_body(std::vformat(error_template_, std::make_format_args(code, message)));
    add_header(header::kContentType, "text/html");
}

Response& Response::add_body(std::string&& body)
{
    if (body.size() > 0) {
        add_header(header::kContentLength, std::to_string(body.size()));
    }
    this->body = std::move(body);
    return *this;
}

Response& Response::add_header(std::string_view key, std::string&& value)
{
    headers.insert_or_assign(std::string(key), std::move(value));
    return *this;
}

std::string Response::to_string() const
{
    std::string raw_response = std::string(kServerHttpVersion) + " " + std::to_string((int)code) + " " + ::to_string(code) + "\r\n";

    for (auto&& [key, value] : headers) {
        raw_response += key + ": " + value + "\r\n";
    }
    raw_response += "\r\n";

    raw_response += body;

    return raw_response;
}
