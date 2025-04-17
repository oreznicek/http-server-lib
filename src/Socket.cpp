#include <unistd.h>
#include <stdexcept>
#include <arpa/inet.h>
#include <cerrno>
#include <iostream>

#include "Socket.hpp"

Socket::Socket(ProtocolFamily prot_fam, SocketType type) {
    constexpr int SELECT_DEFAULT_PROTOCOL = 0;
    socket_fd = socket((int)prot_fam, (int)type, SELECT_DEFAULT_PROTOCOL);
    if (socket_fd == -1) {
        throw std::runtime_error("Couldn't create socket");
    }
}

Socket::Socket(int fd) : socket_fd(fd) {
}

ServerSocket::ServerSocket(ProtocolFamily prot_fam, SocketType type, const SocketAddr& sock_addr) : Socket(prot_fam, type) {
    if (::bind(socket_fd, (const struct sockaddr*)&sock_addr.addr, sizeof(sock_addr.addr)) == -1) {
        perror("error");
        throw std::runtime_error("bind() failed");
    }
    if (::listen(socket_fd, SOMAXCONN) == -1) {
        throw std::runtime_error("listen() failed");
    }
}

ClientSocket ServerSocket::accept_connection(SocketAddr& sock_addr) {
    socklen_t client_addr_size = sizeof(sock_addr.addr);
    int fd = ::accept(socket_fd, (struct sockaddr*)&sock_addr.addr, &client_addr_size);
    if (fd == -1) {
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

Socket::~Socket() {
    if (close(socket_fd) == -1) {
        throw std::runtime_error("close() failed");
    }
}
