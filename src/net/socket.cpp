#include <stdexcept>
#include <cerrno>
#include <cstring>
#include <format>

#include "net/socket.hpp"
#include "logger.hpp"

using namespace net;
using namespace http;

void set_rcv_timeout(socket_t fd, const timeval* timeout) {
    if (!timeout) return;

#ifdef _WIN32
    DWORD timeout_ms = (timeout->tv_sec * 1000) + (timeout->tv_usec / 1000);
    setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout_ms, sizeof(timeout_ms));
#else
    setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, (const void*)timeout, sizeof(timeval));
#endif
}

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
    if (socket_fd_ == kInvalidSocketFd) {
        throw std::runtime_error(std::format("socket() failed: {}", SOCK_ERROR_CODE));
    }

    if (prot == Protocol::Ipv6 || prot == Protocol::DualStack) {
        int on = (prot == Protocol::Ipv6);
        setsockopt(socket_fd_, IPPROTO_IPV6, IPV6_V6ONLY, (const char*)&on, sizeof(on));
    }
}

Socket::Socket(int fd) : socket_fd_(fd)
{}

Socket::Socket(Socket&& other) noexcept
    : socket_fd_(other.socket_fd_)
{
    other.socket_fd_ = kInvalidSocketFd;
}

Socket& Socket::operator=(Socket&& other) noexcept
{
    if (this != &other) {
        close();
        socket_fd_ = other.socket_fd_;
        other.socket_fd_ = kInvalidSocketFd;
    }
    return *this;
}

Socket::~Socket()
{
    if (socket_fd_ != kInvalidSocketFd) {
        close();
    }
}

void Socket::close() noexcept
{
    logger::debug("Socket{{ fd = {} }}.close()", socket_fd_);
    if (CLOSE(socket_fd_) == kSocketError) {
        logger::error("Socket{{ fd = {} }}.close() failed: {}", socket_fd_, SOCK_ERROR_CODE);
    }
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
    if (::bind(socket_fd_, sock_addr.data(), sock_addr.size()) == kSocketError) {
        throw std::runtime_error(std::format("ServerSocket{{ fd = {} }}.bind() failed: {}", socket_fd_, SOCK_ERROR_CODE));
    }
    sockaddr_storage srv_addr;
    socklen_t srv_addr_len = sizeof(srv_addr);
    if (::getsockname(socket_fd_, (sockaddr*)&srv_addr, &srv_addr_len) == kSocketError) {
        throw std::runtime_error(std::format("ServerSocket{{ fd = {} }}.getsockname() failed: {}", socket_fd_, SOCK_ERROR_CODE));
    }

    if (srv_addr.ss_family == AF_INET) {
        srv_port_ = ntohs(((sockaddr_in*)&srv_addr)->sin_port);
    }
    else if (srv_addr.ss_family == AF_INET6) {
        srv_port_ = ntohs(((sockaddr_in6*)&srv_addr)->sin6_port);
    }
    logger::debug("ServerSocket{{ fd = {} }}.srv_port_ = {}", socket_fd_, srv_port_);

    logger::debug("ServerSocket{{ fd = {} }}.listen()", socket_fd_);
    if (::listen(socket_fd_, SOMAXCONN) == kSocketError) {
        throw std::runtime_error(std::format("ServerSocket{{ fd = {} }}.listen() failed: {}", socket_fd_, SOCK_ERROR_CODE));
    }
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
        Socket::operator=(std::move(other));
        pfd_ = other.pfd_;
        srv_port_ = other.srv_port_;
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
        set_rcv_timeout(socket_fd_, timeout);
    }
    logger::debug("ClientSocket{{ fd = {} }}.connect()", socket_fd_);
    if (::connect(socket_fd_, sock_addr.data(), sock_addr.size()) == kSocketError) {
        throw std::runtime_error(std::format("ClientSocket{{ fd = {} }}.connect() failed: {}", socket_fd_, SOCK_ERROR_CODE));
    }
}

ClientSocket::ClientSocket(int fd, const timeval* timeout)
    : Socket(fd)
{
    set_rcv_timeout(socket_fd_, timeout);
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
#ifdef __linux__
    int fds[2];
    if (socketpair(AF_UNIX, SOCK_STREAM, 0, fds) == -1) {
        throw std::runtime_error(std::format("socketpair() failed: {}", SOCK_ERROR_CODE));
    }

    timeval timeout = { .tv_sec = 0, .tv_usec = 3000 };
    ClientSocket dial_sock(fds[0], &timeout);
    ClientSocket answer_sock(fds[1], &timeout);

    return {std::move(dial_sock), std::move(answer_sock)};
#else
    return { ClientSocket(), ClientSocket() };
#endif
}

ClientSocket ServerSocket::poll(SocketAddr& sock_addr, const timeval* timeout)
{
    logger::debug("ServerSocket{{ fd = {} }}.poll()", socket_fd_);
    int poll_result = POLL(&pfd_, 1, kPollTimeout);

    if (poll_result < 0) {
        throw std::runtime_error(std::format("ServerSocket{{ fd = {} }}.poll() failed: {}", socket_fd_, SOCK_ERROR_CODE));
    } else if (poll_result == 0) {
        return ClientSocket();
    }

    if (pfd_.revents & POLLIN) {
        return accept_connection(sock_addr, timeout);
    }
    throw std::runtime_error(std::format("ServerSocket{{ fd = {} }}.poll() failed: Some error event happened", socket_fd_, SOCK_ERROR_CODE));
    return ClientSocket();
}

ClientSocket ServerSocket::accept_connection(SocketAddr& sock_addr, const timeval* timeout) const
{
    socklen_t client_addr_size = sock_addr.size();
    int fd = ::accept(socket_fd_, sock_addr.data(), &client_addr_size);
    if (fd == kInvalidSocketFd) {
        throw std::runtime_error(std::format("ServerSocket{{ fd = {} }}.accept_connection() failed: {}", socket_fd_, SOCK_ERROR_CODE));
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
    if (result == kSocketError) {
        throw std::runtime_error(std::format("ClientSocket{{ fd = {} }}.send() failed: {}", socket_fd_, SOCK_ERROR_CODE));
    }
    logger::debug("ClientSocket{{ fd = {} }}.send()", socket_fd_);
    return result != -1;
}
