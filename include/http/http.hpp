#ifndef _HTTP_HPP
#define _HTTP_HPP

#include <cstdint>
#include <string>
#include <unordered_map>
#include <utility>

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

using Header = std::pair<std::string, std::string>;
using Headers = std::unordered_map<std::string, std::string>;

struct Request {
    RequestMethod method;
    std::string path;
    std::size_t content_length = 0;
    bool keep_alive = true;
    bool close = false;
    bool host = false; // host field present
    std::string body;
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
    Response& add_header(std::string&& key, std::string&& value);
    std::string to_string() const;

    friend class ServerBuilder;
};

RequestMethod to_request_method(std::string_view str);
std::string to_string(StatusCode code);

} // end of `http` namespace

#endif
