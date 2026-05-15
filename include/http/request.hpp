#ifndef _HTTP_REQUEST_HPP
#define _HTTP_REQUEST_HPP

#include <string>
#include <expected>

namespace http {

/// @brief Strongly-typed enumeration of supported HTTP methods.
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

/**
 * @brief Represents the target URI/path of an HTTP request.
 *
 * @details Extracts and stores the requested relative path. In the future,
 *          this class can easily be expanded to also parse and store URL
 *          query parameters (e.g., `?id=5&sort=asc`).
 */
class RequestTarget {
    RequestTarget(std::string&& relative_path);
public:
    /// @brief The parsed relative path (e.g., "api/users" or "index.html").
    std::string relative_path;
    RequestTarget();
    static std::expected<RequestTarget, std::string> from(std::string_view raw_target);
};

/**
 * @brief A fully parsed and validated HTTP request.
 *
 * @details This struct holds all the necessary data extracted from the raw socket
 *          buffer. By pre-computing fields like `content_length` and connection
 *          state flags during the parsing phase, the router and handlers can process
 *          the request with zero-overhead.
 */
struct Request {
    /// @brief The HTTP action requested by the client
    RequestMethod method;
    /// @brief The specific URI path the client is trying to access.
    RequestTarget target;
    /// @brief The size of the request body in bytes.
    std::size_t content_length = 0;
    /// @brief True if the client wants to keep the TCP connection open for subsequent requests.
    bool keep_alive = true;
    /// @brief True if the client explicitly requested to close the TCP connection.
    bool close = false;
    /// @brief True if the mandatory "Host" header was present in the request.
    bool host = false;
    /// @brief The raw payload/body of the request.
    std::string body;
};

} // end of `http` namespace

#endif
