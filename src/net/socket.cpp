#include <sys/socket.h>
#include <unistd.h>
#include <stdexcept>
#include <arpa/inet.h>
#include <cerrno>
#include <iostream>

#include "net/socket.hpp"
#include "net/socket_addr.hpp"

using namespace net;
using namespace http;

constexpr int INVALID_SOCKET_FD = -1;

Socket::Socket(Protocol prot) {
    constexpr int SELECT_DEFAULT_PROTOCOL = 0;

    socket_fd = socket(
        (prot == Protocol::Ipv4) ? AF_INET : AF_INET6,
        SOCK_STREAM,
        SELECT_DEFAULT_PROTOCOL
    );
    if (socket_fd == -1) {
        throw std::runtime_error("Couldn't create socket");
    }

    if (prot == Protocol::Ipv6 || prot == Protocol::DualStack) {
        int on = (prot == Protocol::Ipv6);
        setsockopt(socket_fd, IPPROTO_IPV6, IPV6_V6ONLY, &on, sizeof(on));
    }
}

Socket::Socket(int fd) : socket_fd(fd) {
}

Socket::~Socket() {
    if (socket_fd == INVALID_SOCKET_FD) {
        return;
    }
    this->close();
}

void Socket::close() {
    if (::close(socket_fd) == -1) {
        throw std::runtime_error("close() failed");
    }
}

ServerSocket::ServerSocket() : Socket(INVALID_SOCKET_FD) {
}

ServerSocket::ServerSocket(Protocol prot, SocketAddr&& sock_addr)
    : Socket(prot) {
    if (::bind(socket_fd, sock_addr.data(), sock_addr.size()) == -1) {
        perror("error");
        throw std::runtime_error("bind() failed");
    }
    if (::listen(socket_fd, SOMAXCONN) == -1) {
        throw std::runtime_error("listen() failed");
    }
}

ServerSocket::ServerSocket(SocketAddr4&& sock_addr)
    : ServerSocket(Protocol::Ipv4, std::move(sock_addr)) {
}

ServerSocket::ServerSocket(SocketAddr6&& sock_addr)
    : ServerSocket(Protocol::Ipv6, std::move(sock_addr)) {
}
ServerSocket::ServerSocket(SocketAddr46&& sock_addr)
    : ServerSocket(Protocol::DualStack, std::move(sock_addr)) {
}

ServerSocket& ServerSocket::operator=(ServerSocket&& other) noexcept {
    if (this != &other) {
        socket_fd = other.socket_fd;
        other.socket_fd = INVALID_SOCKET_FD;
    }
    return *this;
}

ClientSocket::ClientSocket(Protocol prot, const SocketAddr& sock_addr, const timeval* timeout)
    : Socket(prot) {
    setsockopt(socket_fd, SOL_SOCKET, SO_RCVTIMEO, (const void*)timeout, sizeof(timeval));
    if (::connect(socket_fd, sock_addr.data(), sock_addr.size()) == -1) {
        throw std::runtime_error("connect() failed");
    }
}

ClientSocket::ClientSocket(int fd, const timeval* timeout) : Socket(fd) {
    setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, (const void*)timeout, sizeof(timeval));
}

ClientSocket::ClientSocket(const SocketAddr4& sock_addr, const timeval* timeout) : ClientSocket(Protocol::Ipv4, sock_addr, timeout) {
}

ClientSocket ServerSocket::accept_connection(SocketAddr& sock_addr, const timeval* timeout) const {
    socklen_t client_addr_size = sock_addr.size();
    int fd = ::accept(socket_fd, sock_addr.data(), &client_addr_size);
    if (fd == -1) {
        perror("error");
        throw std::runtime_error("accept() failed");
    }
    return ClientSocket(fd, timeout);
}

int ClientSocket::read(char* buffer, std::size_t count) {
    return ::recv(socket_fd, buffer, count, 0);
}

// Returns if the write was successful
bool ClientSocket::write(const std::string& buffer) {
    int result = ::write(socket_fd, buffer.c_str(), buffer.size());
    return result != -1;
}
