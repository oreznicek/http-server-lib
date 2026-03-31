#include <stdexcept>

#include "net/socket_addr.hpp"

using namespace net;

SocketAddr4::SocketAddr4()
    : addr(AF_INET, 0, {INADDR_ANY}, {0})
{}

SocketAddr4::SocketAddr4(in_port_t port_number)
    : addr{AF_INET, htons(port_number), {INADDR_ANY}, {0}}
{}

SocketAddr4::SocketAddr4(std::string&& str_addr, in_port_t port_number)
    : SocketAddr4(port_number)
{
    if (inet_pton(AF_INET, str_addr.c_str(), &addr.sin_addr) == -1) {
        std::runtime_error("Invalid address / Address not supported");
    }
}

sockaddr* SocketAddr4::data()
{
    return (sockaddr*)&addr;
}

const sockaddr* SocketAddr4::data() const
{
    return (const sockaddr*)&addr;
}

socklen_t SocketAddr4::size() const
{
    return sizeof(addr);
}

SocketAddr6::SocketAddr6()
    : addr{AF_INET6, 0, 0, in6addr_any, 0}
{}

SocketAddr6::SocketAddr6(in_port_t port_number)
    : addr{AF_INET6, htons(port_number), 0, in6addr_any, 0}
{}

sockaddr* SocketAddr6::data()
{
    return (sockaddr*)&addr;
}

const sockaddr* SocketAddr6::data() const
{
    return (const sockaddr*)&addr;
}

socklen_t SocketAddr6::size() const
{
    return sizeof(addr);
}

SocketAddr46::SocketAddr46(in_port_t port_number)
    : SocketAddr6(port_number)
{}
