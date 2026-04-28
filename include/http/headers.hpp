#ifndef _HTTP_HEADER_HPP
#define _HTTP_HEADER_HPP

#include "http/mime.hpp"

#include <string_view>
#include <unordered_map>

namespace http {

using Header = std::pair<std::string, std::string>;
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
