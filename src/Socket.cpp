#include <unistd.h>
#include <stdexcept>
#include <arpa/inet.h>
#include <cerrno>
#include <iostream>

#include "Socket.hpp"

Socket::Socket(ProtocolFamily prot_fam, SocketType type) {
    std::cout << "Socket constructor running" << std::endl;
    constexpr int SELECT_DEFAULT_PROTOCOL = 0;
    socket_fd = socket((int)prot_fam, (int)type, SELECT_DEFAULT_PROTOCOL);
    if (socket_fd == -1) {
        throw std::runtime_error("Couldn't create socket");
    }
}

Socket::Socket(int fd) : socket_fd(fd) {
    std::cout << "Socket fd constructor" << std::endl;
}

Socket::~Socket() {
    std::cout << "Closed socket: " << socket_fd << std::endl;
    if (close(socket_fd) == -1) {
        throw std::runtime_error("close() failed");
    }
}

ServerSocket::ServerSocket() : Socket(-1) {
}

ServerSocket::ServerSocket(ProtocolFamily prot_fam, SocketType type, SocketAddr&& sock_addr) : Socket(prot_fam == ProtocolFamily::DUAL_STACK ? ProtocolFamily::IPV6 : prot_fam, type) {
    std::cout << "ServerSocket constructor running" << std::endl;
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
    std::cout << "Listening on: " << socket_fd << std::endl;
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

ClientSocket ServerSocket::accept_connection(SocketAddr& sock_addr) const {
    socklen_t client_addr_size = sock_addr.size();
    std::cout << socket_fd << std::endl;
    int fd = ::accept(socket_fd, sock_addr.data(), &client_addr_size);
    if (fd == -1) {
        perror("error");
        throw std::runtime_error("accept() failed");
    }
    return ClientSocket(fd);
}

// Reads the entire client message
std::string ClientSocket::read() {
    constexpr size_t CHUNK = 4;
    std::string request_message;
    int result;

    do {
        request_message.resize(request_message.size() + CHUNK);
        result = ::read(socket_fd, &request_message[request_message.size() - CHUNK], CHUNK); 
    } while (result == CHUNK);

    if (result == -1) {
        perror("error");
        throw std::runtime_error("read() failed");
    } else { // result < CHUNK
        request_message.resize(request_message.size() - CHUNK + result);
    }

    return request_message;
}

// Returns if the write was successful
bool ClientSocket::write(const char* buffer, std::size_t count) {
    int result = ::write(socket_fd, buffer, count);
    return !(result == -1);
}
