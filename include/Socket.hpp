#ifndef _SOCKET_HPP
#define _SOCKET_HPP

#include <sys/socket.h>
#include <netinet/in.h>
#include <string>

#include "SocketAddr.hpp"

enum class ProtocolFamily {
    IPV4 = AF_INET,
    IPV6 = AF_INET6
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
public:
    ServerSocket(ProtocolFamily prot_fam, SocketType type, const SocketAddr& sock_addr);
    ClientSocket accept_connection(SocketAddr& sock_addr);
};

#endif
