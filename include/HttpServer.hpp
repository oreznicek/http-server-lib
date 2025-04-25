#ifndef _HTTP_SERVER_HPP
#define _HTTP_SERVER_HPP

#include <thread>
#include <stdexcept>

#include "Socket.hpp"

class HttpServer;

class HttpServerBuilder {
    std::string public_dir = "public";
    in_port_t port = 8080; 
    bool ipv4 = true;
    bool ipv6 = true;
public:
    HttpServerBuilder& set_public_dir(std::string&& dir);
    HttpServerBuilder& set_port(in_port_t port);
    HttpServerBuilder& enable_ipv4();
    HttpServerBuilder& disable_ipv4();
    HttpServerBuilder& enable_ipv6();
    HttpServerBuilder& disable_ipv6();

    HttpServer build();
    friend class HttpServer;
};
class HttpServer {
    ServerSocket ssock;
    // ClientRequestParser req_parser;
    // public dir
    HttpServer(const HttpServerBuilder& builder);
public:
    HttpServer() = delete;
    void run();

    friend class HttpServerBuilder;
};

#endif
