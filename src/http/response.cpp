#include "http/response.hpp"
#include "http/server.hpp"

using namespace http;

#include <format>

ServerErr::ServerErr(StatusCode code)
    : code(code)
{}

ServerErr::ServerErr(StatusCode code, std::string&& msg)
    : code(code), message(msg)
{}

std::string http::to_string(StatusCode code)
{
    switch (code) {
        case StatusCode::Ok:
            return "OK";
        case StatusCode::Created:
            return "Created";
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

/** @brief Constructs an invalid response. */
Response::Response()
    : code(StatusCode::None), body("")
{}

/** @brief Constructs a response with a specific status code. */
Response::Response(StatusCode code)
    : code(code), body("")
{}

/** @brief Constructs an error response directly from a `ServerErr` object. */
Response::Response(ServerErr err)
    : Response(err.code)
{
    int code = (int)err.code;
    std::string message = err.message.empty() ? ::to_string(err.code) : std::move(err.message);
    add_body(std::vformat(error_template_, std::make_format_args(code, message)));
    add_header(header::kContentType, header::ContentType::Value::TextHtml);
}

/**
 * @brief Appends a payload body to the response.
 * @param body The string data to set as the body.
 * @return A reference to this response.
 */
Response& Response::add_body(std::string&& body)
{
    if (body.size() > 0) {
        add_header(header::kContentLength, std::to_string(body.size()));
    }
    this->body = std::move(body);
    return *this;
}

/**
 * @brief Adds a raw, dynamically typed HTTP header to the response.
 *
 * @details It is highly recommended to use `std::string_view constants
 *          for header keys, defined in `http::header`
 *          (e.g., `http::header::ContentLength`).
 *
 * @param key The header field name.
 * @param value The header value.
 * @return A reference to this response.
 */
Response& Response::add_header(std::string_view key, std::string&& value)
{
    headers.insert_or_assign(std::string(key), std::move(value));
    return *this;
}

/**
 * @brief Serializes the entire response object into a raw network buffer.
 *
 * @details Generates the HTTP Status-Line, appends all headers, inserts the
 *          mandatory blank line (`\r\n\r\n`), and appends the body.
 *
 * @return The fully formatted HTTP string.
 */
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
