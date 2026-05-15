#ifndef _HTTP_RESPONSE_HPP
#define _HTTP_RESPONSE_HPP

#include "http/headers.hpp"

#include <cstdint>
#include <string>

namespace http {

/// @brief Strongly-typed enumeration of standard HTTP status codes.
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

/**
 * @brief Represents an internal server error generated during request processing.
 *
 * @details Primarily used as the error payload in `std::expected` returns
 *          during the parsing phase. It bundles an HTTP status code with
 *          an optional detailed error message for debugging.
 */
struct ServerErr {
    StatusCode code;
    std::string message;
    ServerErr(StatusCode code);
    ServerErr(StatusCode code, std::string&& msg);
};

/**
 * @brief Constructs and serializes outgoing HTTP responses.
 *
 * @details This class utilizes a fluent interface (Builder pattern), allowing
 *          handlers to easily chain methods like `add_header()` and `add_body()`.
 *          Once fully constructed, the `to_string()` method serializes the
 *          object into a valid HTTP-formatted text buffer ready for transmission.
 */
class Response {
private:
    /// @brief A global HTML template used for auto-generating error pages.
    static std::string error_template_;
public:
    /// @brief The HTTP status code to be returned to the client.
    StatusCode code;
    /// @brief The payload body of the response (e.g., HTML, JSON, or binary data).
    std::string body;
    /// @brief The collection of HTTP headers to be sent with the response.
    Headers headers;

    Response();
    Response(StatusCode code);
    Response(ServerErr err);

    Response& add_body(std::string&& body);
    Response& add_header(std::string_view key, std::string&& value);

    /**
     * @brief Adds a strongly-typed HTTP header to the response.
     *
     * @details This is a highly safe, template-driven method that integrates with
     *          the `http::header` structs. It completely eliminates magic strings
     *          and ensures that only valid values can be assigned to specific headers.
     *
     * @tparam HeaderTag The strongly-typed header struct (e.g., `http::header::kContentType`).
     * @param val The safe enumeration value (e.g., `http::header::ContentType::Value::TextHtml`).
     * @return A reference to this response.
     */
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
