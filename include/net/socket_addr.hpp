#ifndef _NET_SOCKET_ADDR_HPP
#define _NET_SOCKET_ADDR_HPP

#include <string>
#include <arpa/inet.h>

namespace net {

class SocketAddr {
    virtual sockaddr* data() = 0;
    virtual const sockaddr* data() const = 0;
    virtual socklen_t size() const = 0;
    friend class ServerSocket;
    friend class ClientSocket;
};

class SocketAddr4 : public SocketAddr {
    sockaddr_in addr;
    sockaddr* data() override;
    const struct sockaddr* data() const override;
    socklen_t size() const override;
public:
    SocketAddr4();
    SocketAddr4(in_port_t port_number);
};

class SocketAddr6 : public SocketAddr {
    sockaddr_in6 addr;
    sockaddr* data() override;
    const sockaddr* data() const override;
    socklen_t size() const override;
public:
    SocketAddr6();
    SocketAddr6(in_port_t port_number);
};

class SocketAddr46 : public SocketAddr6 {
public:
    SocketAddr46(in_port_t port_number);
};

} // end of `net` namespace

#endif
