#ifndef _ROUTER_HPP
#define _ROUTER_HPP

#include "http/request.hpp"
#include "http/response.hpp"
#include <filesystem>
#include <functional>

namespace http {
    class ServerBuilder; // forward declaration
}

namespace handlers {

/**
 * @brief Core HTTP request router and static file server.
 *
 * @details The Router is responsible dispaching incoming HTTP requests to the
 *          appropriate user-defined handler based on the URI path and HTTP method.
 *          If a route is not found in the dynamic handler map, the router
 *          automatically falls back to attempting to serve a static file
 *          from the configured `public_dir_`.
 */
class Router {
public:
    /// @brief The required function signature for all dynamic route handlers.
    using HandlerFunc = std::function<http::Response(const http::Request&)>;
private:
    std::filesystem::path public_dir_ = std::filesystem::current_path();
    bool list_dir_ = false;
    std::unordered_map<std::string, std::unordered_map<http::RequestMethod, HandlerFunc>> routes_;
    http::Response serve_static_file(const std::string& uri) const;
public:
    void add_route(std::string&& path, http::RequestMethod method, HandlerFunc&& func);
    http::Response handle_request(const http::Request& req) const;

    friend class http::ServerBuilder;
};

} // end of `handlers` namespace

#endif
