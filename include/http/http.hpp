#ifndef _HTTP_HPP
#define _HTTP_HPP

#include <cstdint>
#include <string>

namespace http {

enum class RequestMethod {
    Get = 0,
    Post,
    Put,
    Delete,
    None = 999
};

enum class StatusCode : uint16_t {
    // 2xx Success
    Ok = 200,

    // 3xx Redirection

    // 4xx Client Errors
    BadRequest = 400,
    RequestTimeout = 408,
    ContentTooLarge = 413,
    UriTooLong = 414,
    RequestHeaderFieldsTooLarge = 431,

    // 5xx Server Errors
    InternalServerError = 500,
    NotImplemented = 501,
    HttpVersionNotSupported = 505,

    None = 999
};

struct Request {
    RequestMethod method;
    std::string relative_path;
    std::size_t content_length = 0;
    bool keep_alive = true;
    bool close = false;
    bool host = false; // host field present
    std::string body;
};

RequestMethod to_request_method(std::string_view str);
std::string to_string(StatusCode code);

} // end of `http` namespace

#endif
