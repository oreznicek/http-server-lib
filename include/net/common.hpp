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

#define sys_poll WSAPoll
#define sys_close closesocket
#else
    // Your existing POSIX headers
#include <arpa/inet.h>
#include <poll.h>
#include <sys/socket.h>

using socket_t = int;
constexpr socket_t kInvalidSocketFd = -1;
// POSIX defines in_port_t in <arpa/inet.h> header file
using optval_t = const char*;
constexpr int kSocketError = -1;

#define sys_poll ::poll
#define sys_close ::close
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