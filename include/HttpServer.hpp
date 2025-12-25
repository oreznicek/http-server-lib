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
    std::size_t headers_limit = 8 * 1024;
    std::size_t body_limit = 4 * 1024 * 1024;
    timeval timeout{ .tv_sec = 5, .tv_usec = 0 };
public:
    HttpServerBuilder& set_public_dir(std::string&&);
    HttpServerBuilder& set_port(in_port_t);

    HttpServerBuilder& enable_ipv4();
    HttpServerBuilder& disable_ipv4();

    HttpServerBuilder& enable_ipv6();
    HttpServerBuilder& disable_ipv6();

    HttpServerBuilder& set_request_headers_size_limit(std::size_t);
    HttpServerBuilder& set_request_body_size_limit(std::size_t);

    HttpServerBuilder& set_request_timeout(const timeval& timeout);

    HttpServer build();

    friend class HttpServer;
};

class HttpServer {
    ServerSocket ssock;
    HttpRequestParser parser;
    timeval timeout;
    // public dir
    HttpServer(const HttpServerBuilder&);
public:
    HttpServer() = delete;
    void run();

    friend class HttpServerBuilder;
};

void send_error_response(ClientSocket& csock, HttpStatusCode code);

#endif
