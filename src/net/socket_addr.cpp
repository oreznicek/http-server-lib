#include <stdexcept>

#include "net/socket_addr.hpp"
#include "logger.hpp"

using namespace net;

SocketAddr4::SocketAddr4()
    : addr(AF_INET, 0, {INADDR_ANY}, {0})
{}

SocketAddr4::SocketAddr4(in_port_t port_number)
    : addr{AF_INET, htons(port_number), {INADDR_ANY}, {0}}
{}

SocketAddr4::SocketAddr4(const std::string& address, in_port_t port_number)
    : SocketAddr4(port_number)
{
    int result = inet_pton(AF_INET, address.c_str(), &addr.sin_addr);

    if (result == 0) {
        throw std::runtime_error("Invalid IPv4 address format: " + address);
    }
    else if (result < 0) {
        throw std::runtime_error(std::format("inet_pton system error for {} : {}", address, WSAGetLastError()));
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

SocketAddr6::SocketAddr6(const std::string& address, in_port_t port_number)
    : SocketAddr6(port_number)
{
    int result = inet_pton(AF_INET6, address.c_str(), &addr.sin6_addr);

    if (result == 0) {
        throw std::runtime_error("Invalid IPv6 address format: " + address);
    }
    else if (result < 0) {
        throw std::runtime_error(std::format("inet_pton system error for {} : {}", address, WSAGetLastError()));
    }
}

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
