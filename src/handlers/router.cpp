#include "handlers/router.hpp"

#include <sstream>
#include <fstream>
#include <iostream>

using namespace handlers;
using namespace http;
namespace fs = std::filesystem;

static Response generate_directory_listing(const fs::path& dir_path, const std::string& relative_path) {
    std::string html = "<html><head><title>Index of /" + relative_path + "</title></head><body>";
    html += "<h1>Index of /" + relative_path + "</h1><hr><ul>";

    if (relative_path != "") {
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
        .add_header(header::kContentType, header::ContentType::Value::TextHtml)
        .add_body(std::move(html));
}

Response Router::serve_static_file(const std::string& relative_path) const
{
    fs::path target_path = (public_dir_ / relative_path).lexically_normal();

    if (fs::is_directory(target_path)) {
        fs::path index_path = target_path / "index.html";

        if (fs::exists(index_path)) {
            target_path = index_path;
        } else if (list_dir_) {
            return generate_directory_listing(target_path, relative_path);
        }
    }

    std::error_code ec;
    if (!fs::is_regular_file(target_path, ec)) {
        // TODO: Log this and similar errors
        //std::cout << ec.message() << std::endl;
        return Response(ServerErr(StatusCode::NotFound));
    }

    std::ifstream file(target_path, std::ios::binary);
    if (!file.is_open()) {
        return Response(ServerErr(StatusCode::InternalServerError, "Could not open file " + relative_path + " for reading."));
    }

    std::ostringstream buffer;
    buffer << file.rdbuf();
    std::string body = buffer.str();

    return Response(StatusCode::Ok)
        .add_header(header::kContentType, mime::extension_to_type(target_path.extension().native()))
        .add_body(std::move(body));
}

void Router::add_route(std::string&& relative_path, http::RequestMethod method, HandlerFunc&& func)
{
    routes_[std::move(relative_path)][method] = std::move(func);
}

Response Router::handle_request(const Request& req) const
{
    auto path_it = routes_.find(req.target.relative_path);
    if (path_it != routes_.end()) {
        const auto& method_map = path_it->second;
        auto method_it = method_map.find(req.method);
        if (method_it != method_map.end()) {
            return method_it->second(req); // execute custom handler
        }
    }
    // No custom route was found -> fall back to serving static file
    return serve_static_file(req.target.relative_path);
}
