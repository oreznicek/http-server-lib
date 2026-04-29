#include <sys/socket.h>
#include <unistd.h>
#include <stdexcept>
#include <arpa/inet.h>
#include <cerrno>
#include <cstring>
#include <format>

#include "net/socket.hpp"
#include "net/socket_addr.hpp"
#include "logger.hpp"

using namespace net;
using namespace http;

Socket::Socket()
    : socket_fd_(kInvalidSocketFd)
{}

Socket::Socket(Protocol prot)
{
    constexpr int kSelectDefaultProtocol = 0;

    socket_fd_ = socket(
        (prot == Protocol::Ipv4) ? AF_INET : AF_INET6,
        SOCK_STREAM,
        kSelectDefaultProtocol
    );
    if (socket_fd_ == -1) {
        throw std::runtime_error(std::format("socket() failed: {}", strerror(errno)));
    }

    if (prot == Protocol::Ipv6 || prot == Protocol::DualStack) {
        int on = (prot == Protocol::Ipv6);
        setsockopt(socket_fd_, IPPROTO_IPV6, IPV6_V6ONLY, &on, sizeof(on));
    }
}

Socket::Socket(int fd) : socket_fd_(fd)
{}

Socket::Socket(Socket&& other) noexcept
    : socket_fd_(other.socket_fd_)
{
    other.socket_fd_ = kInvalidSocketFd;
}

Socket::~Socket()
{
    if (socket_fd_ != kInvalidSocketFd) {
        close();
    }
}

void Socket::close()
{
    if (::close(socket_fd_) == -1) {
        throw std::runtime_error(std::format("Socket{{ fd = {} }}.close() failed: {}", socket_fd_, strerror(errno)));
    }
    logger::debug("Socket{{ fd = {} }}.close()", socket_fd_);
    socket_fd_ = kInvalidSocketFd;
}


bool Socket::is_valid()
{
    return socket_fd_ != kInvalidSocketFd;
}

ServerSocket::ServerSocket() : Socket()
{}

ServerSocket::ServerSocket(Protocol prot, SocketAddr&& sock_addr)
    : Socket(prot)
{
    pfd_.fd = socket_fd_;
    pfd_.events = POLLIN;
    if (::bind(socket_fd_, sock_addr.data(), sock_addr.size()) == -1) {
        throw std::runtime_error(std::format("ServerSocket{{ fd = {} }}.bind() failed: {}", socket_fd_, strerror(errno)));
    }
    sockaddr_in srv_addr;
    socklen_t srv_addr_len = sizeof(sockaddr);
    if (::getsockname(socket_fd_, (sockaddr*)&srv_addr, &srv_addr_len)) {
        throw std::runtime_error(std::format("ServerSocket{{ fd = {} }}.getsockname() failed: {}", socket_fd_, strerror(errno)));
    }
    srv_port_ = ntohs(srv_addr.sin_port);
    if (::listen(socket_fd_, SOMAXCONN) == -1) {
        throw std::runtime_error(std::format("ServerSocket{{ fd = {} }}.listen() failed: {}", socket_fd_, strerror(errno)));
    }
    logger::debug("ServerSocket{{ fd = {} }}.listen()", socket_fd_);
}

ServerSocket::ServerSocket(SocketAddr4&& sock_addr)
    : ServerSocket(Protocol::Ipv4, std::move(sock_addr))
{}
ServerSocket::ServerSocket(SocketAddr6&& sock_addr)
    : ServerSocket(Protocol::Ipv6, std::move(sock_addr))
{}
ServerSocket::ServerSocket(SocketAddr46&& sock_addr)
    : ServerSocket(Protocol::DualStack, std::move(sock_addr))
{}

ServerSocket& ServerSocket::operator=(ServerSocket&& other) noexcept
{
    if (this != &other) {
        socket_fd_ = other.socket_fd_;
        pfd_ = other.pfd_;
        srv_port_ = other.srv_port_;
        other.socket_fd_ = kInvalidSocketFd;
    }
    return *this;
}

ClientSocket::ClientSocket()
    : Socket()
{}


ClientSocket::ClientSocket(int fd)
    : Socket(fd)
{}

ClientSocket::ClientSocket(Protocol prot, const SocketAddr& sock_addr, const timeval* timeout)
    : Socket(prot)
{
    if (timeout != nullptr) {
        setsockopt(socket_fd_, SOL_SOCKET, SO_RCVTIMEO, (const void*)timeout, sizeof(timeval));
    }
    if (::connect(socket_fd_, sock_addr.data(), sock_addr.size()) == -1) {
        throw std::runtime_error(std::format("ClientSocket{{ fd = {} }}.connect()", socket_fd_));
    }
    logger::debug("ClientSocket{{ fd = {} }}.connect()", socket_fd_);
}

ClientSocket::ClientSocket(int fd, const timeval* timeout)
    : Socket(fd)
{
    setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, (const void*)timeout, sizeof(timeval));
}

ClientSocket::ClientSocket(const SocketAddr4& sock_addr)
    : ClientSocket(Protocol::Ipv4, sock_addr, nullptr)
{}

ClientSocket::ClientSocket(const SocketAddr6& sock_addr)
    : ClientSocket(Protocol::Ipv6, sock_addr, nullptr)
{}

ClientSocket::ClientSocket(const SocketAddr4& sock_addr, const timeval* timeout)
    : ClientSocket(Protocol::Ipv4, sock_addr, timeout)
{}

ClientSocket::ClientSocket(const SocketAddr6& sock_addr, const timeval* timeout)
    : ClientSocket(Protocol::Ipv6, sock_addr, timeout)
{}

std::tuple<ClientSocket, ClientSocket> ClientSocket::create_socketpair()
{
    int fds[2];
    if (socketpair(AF_UNIX, SOCK_STREAM, 0, fds) == -1) {
        throw std::runtime_error(std::format("socketpair() failed: {}", strerror(errno)));
    }

    timeval timeout = { .tv_sec = 0, .tv_usec = 3000 };
    ClientSocket dial_sock(fds[0], &timeout);
    ClientSocket answer_sock(fds[1], &timeout);

    return {std::move(dial_sock), std::move(answer_sock)};
}

ClientSocket ServerSocket::poll(SocketAddr& sock_addr, const timeval* timeout)
{
    int poll_result = ::poll(&pfd_, 1, kPollTimeout);

    if (poll_result < 0) {
        throw std::runtime_error(std::format("ServerSocket{{ fd = {} }}.poll() failed: {}", socket_fd_, strerror(errno)));
    } else if (poll_result == 0) {
        return ClientSocket();
    }

    if (pfd_.revents & POLLIN) {
        return accept_connection(sock_addr, timeout);
    }
    throw std::runtime_error(std::format("ServerSocket{{ fd = {} }}.poll() failed: Some error event happened", socket_fd_, strerror(errno)));
    return ClientSocket();
}

ClientSocket ServerSocket::accept_connection(SocketAddr& sock_addr, const timeval* timeout) const
{
    socklen_t client_addr_size = sock_addr.size();
    int fd = ::accept(socket_fd_, sock_addr.data(), &client_addr_size);
    if (fd == -1) {
        throw std::runtime_error(std::format("ServerSocket{{ fd = {} }}.accept_connection() failed: {}", socket_fd_, strerror(errno)));
    }
    logger::debug("ServerSocket{{ fd = {} }}.accept_connection() -> ClientSocket{{ fd = {} }}", socket_fd_, fd);
    return ClientSocket(fd, timeout);
}

int ClientSocket::recv(char* buffer, std::size_t count)
{
    return ::recv(socket_fd_, buffer, count, 0);
}

// Returns if the write was successful
bool ClientSocket::send(const std::string& buffer)
{
    int result = ::send(socket_fd_, buffer.c_str(), buffer.size(), 0);
    if (result == -1) {
        throw std::runtime_error(std::format("ClientSocket{{ fd = {} }}.send() failed: {}", socket_fd_, strerror(errno)));
    }
    logger::debug("ClientSocket{{ fd = {} }}.send()", socket_fd_);
    return result != -1;
}
