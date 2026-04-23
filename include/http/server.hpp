#ifndef _HTTP_SERVER_HPP
#define _HTTP_SERVER_HPP

#include "http/request_parser.hpp"
#include "net/socket.hpp"
#include "handlers/router.hpp"

#include <filesystem>

namespace http {

inline constexpr in_port_t kSelectRandomPort = 0;
inline constexpr in_port_t kServerDefaultPort = 8080;
inline constexpr std::string_view kServerHttpVersion = "HTTP/1.1";

class Server;

class ServerBuilder {
    std::filesystem::path public_dir_ = std::filesystem::current_path();
    in_port_t port_ = kServerDefaultPort;
    bool ipv4_ = true;
    bool ipv6_ = true;
    bool list_dir_ = false;
    std::size_t headers_limit_ = 8 * 1024;
    std::size_t body_limit_ = 4 * 1024 * 1024;
    std::size_t request_target_limit_ = 8 * 1024;
    timeval timeout_{ .tv_sec = 5, .tv_usec = 0 };
public:
    ServerBuilder& set_public_dir(const std::filesystem::path&);
    ServerBuilder& set_port(in_port_t);

    ServerBuilder& enable_ipv4();
    ServerBuilder& disable_ipv4();

    ServerBuilder& enable_ipv6();
    ServerBuilder& disable_ipv6();

    ServerBuilder& enable_directory_listing();
    ServerBuilder& disable_directory_listing();

    ServerBuilder& set_request_headers_size_limit(std::size_t);
    ServerBuilder& set_request_body_size_limit(std::size_t);
    ServerBuilder& set_request_target_size_limit(std::size_t);

    ServerBuilder& set_request_timeout(const timeval& timeout);

    Server build();

    friend class Server;
};

class Server {
    net::ServerSocket ssock_;
    RequestParser parser_;
    handlers::Router router_;
    timeval timeout_;
    bool is_running_;
    Server(const ServerBuilder&);
public:
    Server() = delete;
    void run();
    void stop();
    net::SocketAddr4 get_addr();
    net::SocketAddr6 get_addr6();

    friend class ServerBuilder;
};

} // end of `http` namespace

#endif
