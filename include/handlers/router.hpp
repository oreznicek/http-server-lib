#ifndef _ROUTER_HPP
#define _ROUTER_HPP

#include "http/http.hpp"
#include <filesystem>
#include <functional>
#include <unordered_map>

namespace handlers {

std::string get_mime_type(const std::filesystem::path& path);
std::string normalize_uri(const std::string& path);

class Router {
public:
    using HandlerFunc = std::function<http::Response(const http::Request&)>;
private:
    std::filesystem::path public_dir_;
    bool list_dir_;
    std::unordered_map<std::string, std::unordered_map<http::RequestMethod, HandlerFunc>> routes_;
    http::Response serve_static_file(const std::string& uri) const;
public:
    Router(std::filesystem::path public_dir, bool list_dir);
    void add_route(std::string&& path, http::RequestMethod method, HandlerFunc&& func);
    http::Response handle_request(const http::Request& req) const;
};

} // end of `handlers` namespace

#endif
