/**
 * @file socket_addr.hpp
 * @brief Object-oriented abstractions for network socket addresses.
 *
 * @details This file provides C++ wrappers around the traditional POSIX C-style
 *          socket address structures (`sockaddr_in`, `sockaddr_in6`).
 */

#ifndef _NET_SOCKET_ADDR_HPP
#define _NET_SOCKET_ADDR_HPP

#include "net/common.hpp"

#include <string>

namespace net {

/**
 * @brief Abstract base class for all network socket addresses.
 *
 * @details This interface defines the contract for accessing the underlying raw
 *          OS-level socket structures. By hiding these methods and granting
 *          friendship to the socket classes, the library safely encapsulates
 *          memory casting and prevents external users from messing with raw pointers.
 */
class SocketAddr {
    virtual sockaddr* data() = 0;
    virtual const sockaddr* data() const = 0;
    virtual socklen_t size() const = 0;
    friend class ServerSocket;
    friend class ClientSocket;
};

/**
 * @brief Represents an IPv4 network address and port.
 *
 * @details A safe wrapper around the POSIX `sockaddr_in` structure.
 */
class SocketAddr4 : public SocketAddr {
    sockaddr_in addr;
    sockaddr* data() override;
    const struct sockaddr* data() const override;
    socklen_t size() const override;
public:
    SocketAddr4();
    SocketAddr4(in_port_t port_number);
    SocketAddr4(const std::string& address, in_port_t port_number);
};

/**
 * @brief Represents an IPv6 network address and port.
 *
 * @details A safe wrapper around the POSIX `sockaddr_in6` structure.
 */
class SocketAddr6 : public SocketAddr {
    sockaddr_in6 addr;
    sockaddr* data() override;
    const sockaddr* data() const override;
    socklen_t size() const override;
public:
    SocketAddr6();
    SocketAddr6(in_port_t port_number);
    SocketAddr6(const std::string& address, in_port_t port_number);
};

/**
 * @brief Represents a Dual-Stack (IPv4-mapped-IPv6) network address.
 *
 * @details Inherits from `SocketAddr6` but configures the underlying structure
 *          to allow the operating system to accept both IPv4 and IPv6 traffic
 *          simultaneously on the same socket.
 */
class SocketAddr46 : public SocketAddr6 {
public:
    SocketAddr46(in_port_t port_number);
};

} // end of `net` namespace

#endif
