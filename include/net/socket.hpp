#ifndef _NET_SOCKET_HPP
#define _NET_SOCKET_HPP

#include <expected>
#include <poll.h>
#include <sys/socket.h>
#include <string>

#include "net/socket_addr.hpp"
#include "http/http.hpp"

namespace net {

enum class Protocol {
    Ipv4,
    Ipv6,
    DualStack
};

/* Wrapper for unix socket */
class Socket {
protected:
    static constexpr int kInvalidSocketFd = -1;
    int socket_fd_;
    Socket();
    Socket(Protocol prot);
    Socket(int fd);
public:
    Socket(Socket&& other) noexcept;
    ~Socket();
    void close();
    bool is_valid();
    friend class ServerSocket;
};

class ClientSocket : public Socket {
    ClientSocket(Protocol prot, const SocketAddr& sock_addr, const timeval* timeout);
public:
    ClientSocket();
    ClientSocket(int fd, const timeval* timeout);
    ClientSocket(const SocketAddr4& sock_addr, const timeval* timeout);

    int recv(char* buffer, std::size_t count);
    bool send(const std::string& buffer);
};

class ServerSocket : public Socket {
    pollfd pfd_;
    ServerSocket();
    ServerSocket(Protocol prot, SocketAddr&& sock_addr);
    ClientSocket accept_connection(SocketAddr& sock_addr, const timeval* timeout) const;
public:
    ServerSocket(SocketAddr4&& sock_addr);
    ServerSocket(SocketAddr6&& sock_addr);
    ServerSocket(SocketAddr46&& sock_addr);

    ServerSocket& operator=(ServerSocket&& other) noexcept;

    ClientSocket poll(SocketAddr& sock_addr, const timeval* timeout);
    friend class HttpServer;
};

} // end of `net` namespace

#endif
