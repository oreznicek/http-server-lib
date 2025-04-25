#ifndef _SOCKET_ADDR_HPP
#define _SOCKET_ADDR_HPP

#include <string>
#include <arpa/inet.h>

class SocketAddr {
    virtual struct sockaddr* data() = 0;
    virtual const struct sockaddr* data() const = 0;
    virtual socklen_t size() const = 0;
    friend class ServerSocket;
};

class SocketAddr4 : public SocketAddr {
    struct sockaddr_in addr;
    struct sockaddr* data() ;
    const struct sockaddr* data() const override;
    socklen_t size() const override;
public:
    SocketAddr4();
    SocketAddr4(const std::string& ipv4_address, in_port_t port_number);
};

class SocketAddr6 : public SocketAddr {
    struct sockaddr_in6 addr;
    struct sockaddr* data() override;
    const struct sockaddr* data() const override;
    socklen_t size() const override;
public:
    SocketAddr6();
    SocketAddr6(const std::string& ipv6_address, in_port_t port_number);
};

class SocketAddr46 : public SocketAddr6 {
public:
    SocketAddr46(const std::string& ipv6_address, in_port_t port_number);
};

#endif
