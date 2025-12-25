#ifndef _HTTP_HPP
#define _HTTP_HPP

namespace http {

enum class RequestMethod {
    GET = 0,
    POST,
    PUT
    DELETE,
    NONE
};

enum class StatusCode {
    BadRequest = 400,
    RequestHeaderFieldsTooLarge = 431
};

struct Request {
    HttpRequestMethod method;
    std::string relative_path;
    std::size_t content_length = 0;
    bool keep_alive = true;
    std::string body;
};

HttpRequestMethod to_http_request_method(std::string_view str);
std::string to_string(HttpStatusCode code);

} // end of `http` namespace

#endif
