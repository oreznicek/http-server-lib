#ifndef _HTTP_SERVER_HPP
#define _HTTP_SERVER_HPP

#include "http/request_parser.hpp"
#include "net/socket.hpp"
#include "handlers/router.hpp"
#include "concurrent/thread_pool.hpp"

#include <filesystem>
#include <thread>

namespace http {

namespace detail {
    inline constexpr std::size_t kKilobyte = 1024;
    inline constexpr std::size_t kMegabyte = 1024 * kKilobyte;
} // end of `detail` namespace

inline constexpr std::size_t kDefaultHeadersLimit = 8 * detail::kKilobyte;
inline constexpr std::size_t kDefaultBodyLimit = 4 * detail::kMegabyte;
inline constexpr std::size_t kDefaultRequestTargetLimit = 8 * detail::kKilobyte;
inline constexpr time_t kDefaultTimeoutSeconds = 5;

inline constexpr in_port_t kSelectRandomPort = 0;
inline constexpr in_port_t kServerDefaultPort = 8080;
inline constexpr std::string_view kServerHttpVersion = "HTTP/1.1";

class Server;

class ServerBuilder {
    handlers::Router router_;
    in_port_t port_ = kServerDefaultPort;
    bool ipv4_ = true;
    bool ipv6_ = true;
    std::size_t headers_limit_ = kDefaultHeadersLimit;
    std::size_t body_limit_ = kDefaultBodyLimit;
    std::size_t request_target_limit_ = kDefaultRequestTargetLimit;
    timeval timeout_{ .tv_sec = kDefaultTimeoutSeconds, .tv_usec = 0 };
    std::size_t thread_count_ = std::thread::hardware_concurrency();
public:
    ServerBuilder& set_public_dir(const std::filesystem::path&);
    ServerBuilder& set_error_page_template(std::string&&);
    ServerBuilder& enable_directory_listing();
    ServerBuilder& disable_directory_listing();

    ServerBuilder& add_route(std::string&& path, http::RequestMethod method, handlers::Router::HandlerFunc&& func);

    ServerBuilder& set_port(in_port_t);

    ServerBuilder& enable_ipv4();
    ServerBuilder& disable_ipv4();

    ServerBuilder& enable_ipv6();
    ServerBuilder& disable_ipv6();

    ServerBuilder& set_request_headers_size_limit(std::size_t);
    ServerBuilder& set_request_body_size_limit(std::size_t);
    ServerBuilder& set_request_target_size_limit(std::size_t);

    ServerBuilder& set_request_timeout(const timeval& timeout);

    ServerBuilder& set_thread_count(std::size_t);

    Server build();

    friend class Server;
};

class Server {
    net::ServerSocket ssock_;
    RequestParser parser_;
    handlers::Router router_;
    concurrent::ThreadPool pool_;
    timeval timeout_;
    bool is_running_;
    Server(const ServerBuilder&);
    void handle_client(Connection&& conn);
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
