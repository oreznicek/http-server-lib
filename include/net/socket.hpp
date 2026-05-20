#ifndef _NET_SOCKET_HPP
#define _NET_SOCKET_HPP

#include <string>

#include "net/common.hpp"
#include "net/socket_addr.hpp"

namespace http {
    class Server; // forward declaration
}

namespace net {

/**
 * @brief Specifies the underlying network protocol family to use.
 */
enum class Protocol {
    Ipv4,      ///< Use IPv4 only.
    Ipv6,      ///< Use IPv6 only.
    DualStack  ///< Use a dual-stack socket capable of both IPv4 and IPv6.
};

/**
 * @brief Base class for socket resource management.
 *
 * @details Socket is a move-only type, meaning ownership of the underlying
 *          socket can be transferred safely without double-closing
 *          the file descriptor.
 */
class Socket {
protected:
    /// @brief The underlying OS socket file descriptor.
    socket_t socket_fd_;
    Socket();
    Socket(Protocol prot);
    Socket(socket_t fd);
    void close() noexcept;
public:
    Socket(Socket&& other) noexcept;
    Socket& operator=(Socket&& other) noexcept;
    ~Socket();
    bool is_valid();
    friend class ServerSocket;
};

/**
 * @brief Represents an active, connected endpoint for network communication.
 *
 * @details Used to send and receive data over the network. It can be instantiated
 *          directly to dial a remote server, or returned by a `ServerSocket`
 *          when accepting an incoming connection.
 */
class ClientSocket : public Socket {
    ClientSocket(int fd);
    ClientSocket(Protocol prot, const SocketAddr& sock_addr, const timeval* timeout);
public:
    ClientSocket();
    ClientSocket(int fd, const timeval* timeout);
    ClientSocket(const SocketAddr4& sock_addr);
    ClientSocket(const SocketAddr6& sock_addr);
    ClientSocket(const SocketAddr4& sock_addr, const timeval* timeout);
    ClientSocket(const SocketAddr6& sock_addr, const timeval* timeout);

    static std::tuple<ClientSocket, ClientSocket> create_socketpair();

    int recv(char* buffer, std::size_t count);
    bool send(const std::string& buffer);
};

/**
 * @brief Represents a passive socket that listens for incoming connections.
 *
 * @details Binds to a specific port and IP address, acting as the entry point
 *          for an HTTP server. Uses non-blocking mechanisms like `poll()` to
 *          safely wait for client connections.
 */
class ServerSocket : public Socket {
    static constexpr int kPollTimeout = 100; // ms
    pollfd pfd_;
    in_port_t srv_port_;

    ServerSocket();
    ServerSocket(Protocol prot, SocketAddr&& sock_addr);
    ClientSocket accept_connection(SocketAddr& sock_addr, const timeval* timeout) const;
public:
    ServerSocket(SocketAddr4&& sock_addr);
    ServerSocket(SocketAddr6&& sock_addr);
    ServerSocket(SocketAddr46&& sock_addr);

    ServerSocket& operator=(ServerSocket&& other) noexcept;

    ClientSocket poll(SocketAddr& sock_addr, const timeval* timeout);

    friend class http::Server;
};

} // end of `net` namespace

#endif
