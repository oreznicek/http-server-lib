/**
 * @file headers.hpp
 * @brief Definitions and constants for HTTP headers.
 *
 * @par Cookbook: How to Create a Strongly-Typed Header
 * To eliminate "magic strings" and prevent typos when setting HTTP headers,
 * this library uses a strongly-typed struct pattern. If you need to add
 * a new header, with finite set of values, follow this exact blueprint:
 *
 * 1. Define a `struct` inside the `http::header` namespace.
 * 2. Add a `static constexpr std::string_view name` representing the exact, lowercase HTTP field name.
 * 3. Define a `Value` type (usually an `enum class` to restrict valid user inputs).
 * 4. Provide a `static std::string to_string(Value v)` function to serialize the value for the network.
 *
 * @par Example Implementation:
 * @code
 * struct NewHeader {
 *     static constexpr std::string_view name = "new-header";
 *     enum class Value { Val1, Val2, Val3 };
 *
 *     static std::string to_string(Value v) {
 *         switch (v) {
 *             case Value::Val1: return "value1";
 *             case Value::Val2: return "value2";
 *             case Value::Val3: return "value3";
 *         }
 *     }
 * };
 *
 * // Expose a global instance for the builder API
 * constexpr CacheControl kCacheControl;
 * @endcode
 */
#ifndef _HTTP_HEADERS_HPP
#define _HTTP_HEADERS_HPP

#include "http/mime.hpp"

#include <string_view>
#include <unordered_map>

namespace http {

/// @brief Represents a single HTTP header key-value pair.
using Header = std::pair<std::string, std::string>;
/// @brief A collection of HTTP headers mapped by their lowercase field names.
using Headers = std::unordered_map<std::string, std::string>;

namespace header {
    struct ContentType {
        static constexpr std::string_view name = "content-type";
        using Value = mime::Type;
        static std::string to_string(Value v)
        {
            return mime::to_string(v);
        }
    };

    struct Connection {
        static constexpr std::string_view name = "connection";
        enum class Value { KeepAlive, Close };
        static std::string to_string(Value v)
        {
            switch (v) {
                case Value::KeepAlive: return std::string(kKeepAlive);
            case Value::Close: return std::string(kClose);
            }
        }
        static constexpr std::string_view kKeepAlive = "keep-alive";
        static constexpr std::string_view kClose = "close";
    };

    constexpr std::string_view kHost = "host";
    constexpr std::string_view kContentLength = "content-length";
    constexpr ContentType kContentType;
    constexpr Connection kConnection;
} // end of `http::header` namespace

} // end of `http` namespace

#endif
