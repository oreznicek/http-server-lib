#ifndef _NET_COMMON_HPP
#define _NET_COMMON_HPP

#include <stdexcept>

#ifdef _WIN32
    // Windows requires these headers
#include <winsock2.h>
#include <ws2tcpip.h>

using socket_t = SOCKET;
constexpr socket_t kInvalidSocketFd = INVALID_SOCKET;
using in_port_t = std::uint16_t;
using optval_t = const char*;
constexpr int kSocketError = SOCKET_ERROR;

#define POLL WSAPoll
#define CLOSE closesocket
#define SOCK_ERROR_CODE WSAGetLastError()
#else
    // Your existing POSIX headers
#include <arpa/inet.h>
#include <cerrno>
#include <poll.h>
#include <sys/socket.h>
#include <unistd.h>

using socket_t = int;
constexpr socket_t kInvalidSocketFd = -1;
// POSIX defines in_port_t in <arpa/inet.h> header file
using optval_t = const char*;
constexpr int kSocketError = -1;

#define POLL ::poll
#define CLOSE ::close
#define SOCK_ERROR_CODE errno
#endif

namespace net {

struct Context {
    Context() {
#ifdef _WIN32
        WSADATA wsaData;
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
            throw std::runtime_error("Failed to initialize Winsock");
        }
#endif
    }
    ~Context() {
#ifdef _WIN32
        WSACleanup();
#endif
    }
};

} // end of `net` namespace

#endif
