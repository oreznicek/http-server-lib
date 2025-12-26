#ifndef _HTTP_HPP
#define _HTTP_HPP

#include <string>

namespace http {

enum class RequestMethod {
    GET = 0,
    POST,
    PUT,
    DELETE,
    NONE
};

enum class StatusCode {
    BAD_REQUEST = 400,
    REQUEST_TIMEOUT = 408,
    REQUEST_HEADER_FIELDS_TOO_LARGE = 431
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
