#ifndef _HTTP_RESPONSE_HPP
#define _HTTP_RESPONSE_HPP

#include "http/headers.hpp"

#include <cstdint>
#include <string>

namespace http {

enum class StatusCode : uint16_t {
    // 2xx Success
    Ok = 200,
    Created = 201,

    // 3xx Redirection

    // 4xx Client Errors
    BadRequest = 400,
    NotFound = 404,
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

struct ServerErr {
    StatusCode code;
    std::string message;
    ServerErr(StatusCode code);
    ServerErr(StatusCode code, std::string&& msg);
};

class Response {
private:
    static std::string error_template_;
public:
    StatusCode code;
    std::string body;
    Headers headers;

    Response();
    Response(StatusCode code);
    Response(ServerErr err);

    Response& add_body(std::string&& body);
    Response& add_header(std::string_view key, std::string&& value);

    template <typename HeaderTag>
    Response& add_header(HeaderTag, typename HeaderTag::Value val) {
        headers.insert_or_assign(
            std::string(HeaderTag::name), 
            std::string(HeaderTag::to_string(val))
        );
        return *this;
    }

    std::string to_string() const;

    friend class ServerBuilder;
};

std::string to_string(StatusCode code);

} // end of `http` namespace

#endif
