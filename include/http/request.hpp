#ifndef _HTTP_REQUEST_HPP
#define _HTTP_REQUEST_HPP

#include <string>
#include <expected>

namespace http {

enum class RequestMethod {
    Get = 0,
    Post,
    Put,
    Delete,
    None = 999
};

namespace method {
    inline constexpr std::string_view kGet = "GET";
    inline constexpr std::string_view kPost = "POST";
    inline constexpr std::string_view kPut = "PUT";
    inline constexpr std::string_view kDelete = "DELETE";

    RequestMethod from_string(std::string_view str);
    std::string to_string(RequestMethod method);
} // end of `method` namespace

class RequestTarget {
    RequestTarget(std::string&& relative_path);
public:
    std::string relative_path;
    RequestTarget();
    static std::expected<RequestTarget, std::string> from(std::string_view raw_target);
};

struct Request {
    RequestMethod method;
    RequestTarget target;
    std::size_t content_length = 0;
    bool keep_alive = true;
    bool close = false;
    bool host = false; // host field present
    std::string body;
};

} // end of `http` namespace

#endif
