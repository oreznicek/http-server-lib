#ifndef _NET_SOCKET_HPP
#define _NET_SOCKET_HPP

#include <string>

#include "net/common.hpp"
#include "net/socket_addr.hpp"

namespace http {
class Server; // forward declaration
}

namespace net {

enum class Protocol {
    Ipv4,
    Ipv6,
    DualStack
};

/* Wrapper for unix socket */
class Socket {
protected:
    socket_t socket_fd_;
    Socket();
    Socket(Protocol prot);
    Socket(int fd);
    void close() noexcept;
public:
    Socket(Socket&& other) noexcept;
    Socket& operator=(Socket&& other) noexcept;
    ~Socket();
    bool is_valid();
    friend class ServerSocket;
};

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
