#include "SocketAddr.hpp"

SocketAddr::SocketAddr() : addr(0, 0, {0}) {
}

SocketAddr::SocketAddr(const std::string& ipv4_address, in_port_t port_number) {
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port_number);
    if (inet_pton(AF_INET, ipv4_address.c_str(), &addr.sin_addr) <= 0) {
        throw std::runtime_error("inet_pton() failed");
    }
}

SocketAddr6::SocketAddr6(const std::string& ipv6_address, in_port_t port_number) {
    addr.sin6_family = AF_INET6;
    addr.sin6_port = htons(port_number);
    if (inet_pton(AF_NET6, ipv6_address.c_str(), &addr.sin6_addr) <= 0) {
        throw std::runtime_error("inet_pton() failed");
    }
    addr.sin6_flow_info = 0; // used for QoS -> we don't need that
    addr.sin6_scope_id = 0;
}
