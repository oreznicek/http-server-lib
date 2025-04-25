#ifndef _SOCKET_HPP
#define _SOCKET_HPP

#include <sys/socket.h>
#include <string>

#include "SocketAddr.hpp"

enum class ProtocolFamily {
    IPV4 = AF_INET,
    IPV6 = AF_INET6,
    DUAL_STACK
};

enum class SocketType {
    STREAM = SOCK_STREAM, // reliable
    DATAGRAM = SOCK_DGRAM, // connection-less
    SEQPACKET = SOCK_SEQPACKET
};

/* Wrapper for unix socket */
class Socket {
protected:
    int socket_fd;
    Socket(ProtocolFamily prot_fam, SocketType type);
    Socket(int fd);
public:
    ~Socket();
    friend class ServerSocket;
};

class ClientSocket : public Socket {
public:
    // TODO: take timeout so we don't wait forever for client message
    std::string read();
    bool write(const char* buffer, std::size_t count = 1024);
};

class ServerSocket : public Socket {
    ServerSocket();
    ServerSocket(ProtocolFamily prot_fam, SocketType type, SocketAddr&& sock_addr);
public:
    ServerSocket(SocketType type, SocketAddr4&& sock_addr);
    ServerSocket(SocketType type, SocketAddr6&& sock_addr);
    ServerSocket(SocketType type, SocketAddr46&& sock_addr);
    ClientSocket accept_connection(SocketAddr& sock_addr) const;
    friend class HttpServer;
};

#endif
