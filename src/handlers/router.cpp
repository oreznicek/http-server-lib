#include "handlers/router.hpp"

#include <sstream>
#include <fstream>
#include <iostream>

using namespace handlers;
using namespace http;
namespace fs = std::filesystem;

std::string handlers::get_mime_type(const fs::path& path)
{
    if (path.extension() == ".html") return "text/html";
    if (path.extension() == ".css")  return "text/css";
    if (path.extension() == ".js")   return "application/javascript";
    if (path.extension() == ".png")  return "image/png";
    if (path.extension() == ".jpg" || path.extension() == ".jpeg") return "image/jpeg";
    if (path.extension() == ".txt")  return "text/plain";
    return "application/octet-stream";
}

std::string handlers::normalize_uri(const std::string& path)
{
    std::vector<std::string> parts;
    std::string token;
    std::istringstream token_stream(path);

    while (std::getline(token_stream, token, '/')) {
        if (token == "" || token == ".") {
            continue;
        } else if (token == "..") {
            if (!parts.empty()) {
                parts.pop_back();
            }
        } else {
            parts.push_back(token);
        }
    }

    if (parts.empty()) {
        return "/";
    }

    std::string normalized;
    for (const auto& part : parts) {
        normalized += "/" + part;
    }

    return normalized;
}

Response generate_directory_listing(const fs::path& dir_path, const std::string& uri) {
    std::string html = "<html><head><title>Index of " + uri + "</title></head><body>";
    html += "<h1>Index of " + uri + "</h1><hr><ul>";

    if (uri != "/") {
        html += "<li><a href=\"../\">../ (Parent Directory)</a></li>";
    }

    for (const auto& entry : fs::directory_iterator(dir_path)) {
        std::string filename = entry.path().filename().string();
        std::string href = filename;

        // Append a slash to directories for visual clarity and correct relative linking
        if (entry.is_directory()) {
            filename += "/";
            href += "/";
        }

        html += "<li><a href=\"" + href + "\">" + filename + "</a></li>";
    }

    html += "</ul><hr></body></html>";

    return Response(StatusCode::Ok)
        .add_header(header::kContentType, "text/html")
        .add_body(std::move(html));
}

Response Router::serve_static_file(const std::string& uri) const
{
    std::string relative_uri = uri;
    if (!relative_uri.empty() && relative_uri[0] == '/') {
        relative_uri = relative_uri.substr(1);
    }

    fs::path target_path = (public_dir_ / relative_uri).lexically_normal();

    if (fs::is_directory(target_path)) {
        fs::path index_path = target_path / "index.html";

        if (fs::exists(index_path)) {
            target_path = index_path;
        } else if (list_dir_) {
            return generate_directory_listing(target_path, uri);
        }
    }

    if (!fs::is_regular_file(target_path)) {
        return Response(ServerErr(StatusCode::NotFound));
    }

    std::ifstream file(target_path, std::ios::binary);
    if (!file.is_open()) {
        return Response(ServerErr(StatusCode::InternalServerError, "Could not open file " + relative_uri + " for reading."));
    }

    std::ostringstream buffer;
    buffer << file.rdbuf();
    std::string body = buffer.str();

    return Response(StatusCode::Ok)
        .add_header(header::kContentType, get_mime_type(target_path))
        .add_body(std::move(body));
}

void Router::add_route(std::string&& path, http::RequestMethod method, HandlerFunc&& func)
{
    std::string clean_path = normalize_uri(path);
    routes_[std::move(clean_path)][method] = std::move(func);
}

Response Router::handle_request(const Request& req) const
{
    auto path_it = routes_.find(req.path);
    if (path_it != routes_.end()) {
        const auto& method_map = path_it->second;
        auto method_it = method_map.find(req.method);
        if (method_it != method_map.end()) {
            return method_it->second(req); // execute custom handler
        }
    }
    // No custom route was found -> fall back to serving static file
    return serve_static_file(req.path);
}
