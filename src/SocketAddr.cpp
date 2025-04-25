#include <stdexcept>

#include "SocketAddr.hpp"

SocketAddr4::SocketAddr4() : addr(0, 0, {0}) {
}

SocketAddr4::SocketAddr4(const std::string& ipv4_address, in_port_t port_number) {
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port_number);
    if (inet_pton(AF_INET, ipv4_address.c_str(), &addr.sin_addr) <= 0) {
        throw std::runtime_error("inet_pton() failed");
    }
}

struct sockaddr* SocketAddr4::data() {
    return (sockaddr*)&addr;
}

const struct sockaddr* SocketAddr4::data() const {
    return (const sockaddr*)&addr;
}

socklen_t SocketAddr4::size() const {
    return sizeof(addr);
} 

SocketAddr6::SocketAddr6() : addr(0, 0, {0}) {
}

SocketAddr6::SocketAddr6(const std::string& ipv6_address, in_port_t port_number) {
    addr.sin6_family = AF_INET6;
    addr.sin6_port = htons(port_number);
    if (inet_pton(AF_INET6, ipv6_address.c_str(), &addr.sin6_addr) <= 0) {
        throw std::runtime_error("inet_pton() failed");
    }
    addr.sin6_flowinfo = 0; // used for QoS -> we don't need that
    addr.sin6_scope_id = 0;
}

struct sockaddr* SocketAddr6::data() {
    return (sockaddr*)&addr;
}

const struct sockaddr* SocketAddr6::data() const {
    return (const sockaddr*)&addr;
}

socklen_t SocketAddr6::size() const {
    return sizeof(addr);
} 

SocketAddr46::SocketAddr46(const std::string& ipv6_address, in_port_t port_number) : SocketAddr6(ipv6_address, port_number) {
}
