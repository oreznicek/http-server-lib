#ifndef _SOCKET_ADDR_HPP
#define _SOCKET_ADDR_HPP

#include <string>

class SocketAddr {
    struct sockaddr_in addr;
public:
    SocketAddr();
    SocketAddr(const std::string& ipv4_address, in_port_t port_number);
    friend class ServerSocket;
};

class SocketAddr6 {
    struct sockaddr_in6 addr;
public:
    SocketAddr6(const std::string& ipv6_address, in_port_t port_number);
};

#endif
