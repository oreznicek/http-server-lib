#include <unistd.h>
#include <stdexcept>
#include <arpa/inet.h>
#include <cerrno>
#include <iostream>

#include "Socket.hpp"

constexpr int INVALID_SOCKET_FD = -1;

Socket::Socket(ProtocolFamily prot_fam, SocketType type) {
    constexpr int SELECT_DEFAULT_PROTOCOL = 0;
    socket_fd = socket((int)prot_fam, (int)type, SELECT_DEFAULT_PROTOCOL);
    if (socket_fd == -1) {
        throw std::runtime_error("Couldn't create socket");
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

ServerSocket::ServerSocket(ProtocolFamily prot_fam, SocketType type, SocketAddr&& sock_addr) : Socket(prot_fam == ProtocolFamily::DUAL_STACK ? ProtocolFamily::IPV6 : prot_fam, type) {
    if (prot_fam == ProtocolFamily::IPV6) {
        int on = 1;
        setsockopt(socket_fd, IPPROTO_IPV6, IPV6_V6ONLY, &on, sizeof(on));
    }
    if (::bind(socket_fd, sock_addr.data(), sock_addr.size()) == -1) {
        perror("error");
        throw std::runtime_error("bind() failed");
    }
    if (::listen(socket_fd, SOMAXCONN) == -1) {
        throw std::runtime_error("listen() failed");
    }
}

ServerSocket::ServerSocket(SocketType type, SocketAddr4&& sock_addr) : ServerSocket(ProtocolFamily::IPV4, type, std::move(sock_addr)) {
    std::cout << "Creating IPV4 server socket" << std::endl;
}

ServerSocket::ServerSocket(SocketType type, SocketAddr6&& sock_addr) : ServerSocket(ProtocolFamily::IPV6, type, std::move(sock_addr)) {
    std::cout << "Creating IPV6 server socket" << std::endl;
}
ServerSocket::ServerSocket(SocketType type, SocketAddr46&& sock_addr) : ServerSocket(ProtocolFamily::DUAL_STACK, type, std::move(sock_addr)) {
    std::cout << "Creating dual-stack server socket" << std::endl;
}

ServerSocket& ServerSocket::operator=(ServerSocket&& other) noexcept {
    if (this != &other) {
        socket_fd = other.socket_fd;
        other.socket_fd = INVALID_SOCKET_FD;
    }
    return *this;
}

ClientSocket::ClientSocket(int fd, const timeval* timeout) : Socket(fd) {
    setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, (const void*)timeout, sizeof(timeout));
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

int ClientSocket::read(char* buffer, std::size count) {
    return ::read(socket_fd, buffer, count);
}

// Returns if the write was successful
bool ClientSocket::write(const char* buffer, std::size_t count) {
    int result = ::write(socket_fd, buffer, count);
    return !(result == -1);
}
