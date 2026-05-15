#ifndef _HTTP_CONNECTION_HPP
#define _HTTP_CONNECTION_HPP

#include "http/response.hpp"
#include "net/socket.hpp"

#include <expected>
#include <string>
#include <string_view>

namespace http {

/**
 * @brief Manages an active HTTP client connection.
 *
 * @details This class wraps a raw `net::ClientSocket` and provides buffered,
 *          stream-like reading capabilities. It internally manages leftover
 *          bytes between read operations, allowing callers to safely parse
 *          HTTP headers (using string delimiters) and HTTP bodies (using
 *          fixed byte counts) without losing any incoming data.
 */
class Connection {
    static constexpr int CHUNK = 1024;
    net::ClientSocket csock_;
    std::string leftover_;
    StatusCode read_chunk(std::string& buffer);
public:
    Connection();
    Connection(net::ClientSocket&& csock);
    Connection(Connection&& other) noexcept = default;
    Connection& operator=(Connection&& other) noexcept = default;
    std::expected<std::string, StatusCode> read_until(std::string_view delimiter, std::size_t max_bytes);
    std::expected<std::string, StatusCode> read(std::size_t bytes);
    bool send(const std::string& buffer);
};

} // end of `http` namespace

#endif
