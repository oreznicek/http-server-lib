#ifndef _HTTP_SERVER_HPP
#define _HTTP_SERVER_HPP

#include "http/request_parser.hpp"
#include "net/socket.hpp"

namespace http {

class Server;

class ServerBuilder {
    std::string public_dir = "public";
    in_port_t port = 8080;
    bool ipv4 = true;
    bool ipv6 = true;
    std::size_t headers_limit = 8 * 1024;
    std::size_t body_limit = 4 * 1024 * 1024;
    timeval timeout{ .tv_sec = 5, .tv_usec = 0 };
public:
    ServerBuilder& set_public_dir(std::string&&);
    ServerBuilder& set_port(in_port_t);

    ServerBuilder& enable_ipv4();
    ServerBuilder& disable_ipv4();

    ServerBuilder& enable_ipv6();
    ServerBuilder& disable_ipv6();

    ServerBuilder& set_request_headers_size_limit(std::size_t);
    ServerBuilder& set_request_body_size_limit(std::size_t);

    ServerBuilder& set_request_timeout(const timeval& timeout);

    Server build();

    friend class Server;
};

class Server {
    net::ServerSocket ssock;
    RequestParser parser;
    timeval timeout;
    // public dir
    Server(const ServerBuilder&);
public:
    Server() = delete;
    void run();

    friend class ServerBuilder;
};

} // end of `http` namespace

#endif
