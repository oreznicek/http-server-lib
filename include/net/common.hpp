/**
 * @file common.hpp
 * @brief Cross-platform networking definitions and socket abstractions.
 *
 * @details This file provides a unified abstraction layer over Windows (Winsock2)
 *          and POSIX (Linux/macOS) networking APIs. By defining unified types
 *          (`socket_t`, `in_port_t`) and macros (`POLL`, `CLOSE`, `SOCK_ERROR_CODE`),
 *          the rest of the networking library can be written using a single,
 *          platform-agnostic codebase.
 */

#ifndef _NET_COMMON_HPP
#define _NET_COMMON_HPP

#include <stdexcept>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>

/// @brief Unified socket file descriptor type (SOCKET on Windows, int on POSIX).
using socket_t = SOCKET;
/// @brief Represents an uninitialized or failed socket file descriptor.
constexpr socket_t kInvalidSocketFd = INVALID_SOCKET;
/// @brief Unified type for network ports.
using in_port_t = std::uint16_t;
/// @brief Type definition for socket option values.
using optval_t = const char*;
/// @brief Standardized error return value for socket operations.
constexpr int kSocketError = SOCKET_ERROR;

/// @brief Cross-platform macro for the poll() function.
#define POLL WSAPoll
/// @brief Cross-platform macro to safely close a socket descriptor.
#define CLOSE closesocket
/// @brief Cross-platform macro to retrieve the last socket error code.
#define SOCK_ERROR_CODE WSAGetLastError()
#else
#include <arpa/inet.h>
#include <cerrno>
#include <poll.h>
#include <sys/socket.h>
#include <unistd.h>

/// @brief Unified socket file descriptor type (SOCKET on Windows, int on POSIX).
using socket_t = int;
/// @brief Represents an uninitialized or failed socket file descriptor.
constexpr socket_t kInvalidSocketFd = -1;
// POSIX defines in_port_t in <arpa/inet.h> header file
/// @brief Type definition for socket option values.
using optval_t = const char*;
/// @brief Standardized error return value for socket operations.
constexpr int kSocketError = -1;

/// @brief Cross-platform macro for the poll() function.
#define POLL ::poll
/// @brief Cross-platform macro to safely close a socket descriptor.
#define CLOSE ::close
/// @brief Cross-platform macro to retrieve the last socket error code.
#define SOCK_ERROR_CODE errno
#endif

namespace net {

/**
 * @brief Wrapper for global networking context initialization.
 *
 * @details On Windows, the Winsock API requires global initialization via `WSAStartup`
 *          before any socket functions can be used, and `WSACleanup` when finished.
 *          Instantiating this class at the very beginning of `main()` guarantees
 *          that the network stack is safely initialized and automatically torn down
 *          when the program exits.
 *
 *          On POSIX systems (Linux/macOS), this struct does nothing and compiles
 *          to a zero-overhead no-op.
 */
struct Context {
    /**
     * @brief Initializes the underlying network subsystem.
     * @throw std::runtime_error if Winsock fails to initialize (Windows only).
     */
    Context() {
#ifdef _WIN32
        WSADATA wsaData;
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
            throw std::runtime_error("Failed to initialize Winsock");
        }
#endif
    }

    /**
     * @brief Cleans up and safely tears down the underlying network subsystem.
     */
    ~Context() {
#ifdef _WIN32
        WSACleanup();
#endif
    }
};

} // end of `net` namespace

#endif
