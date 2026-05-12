#include <http/server.hpp>
#include <logger.hpp>

#include <filesystem>
#include <format>
#include <mutex>
#include <source_location>
#include <string>
#include <vector>

namespace fs = std::filesystem;

static fs::path public_dir = fs::path(std::source_location::current().file_name()).remove_filename() / "public";

static std::vector<std::string> database; // In-Memory DB
static std::mutex db_mutex;

static http::Response get_items_handler(const http::Request& req)
{
    std::lock_guard<std::mutex> lock(db_mutex);

    std::string json = "[";
    for (size_t i = 0; i < database.size(); ++i) {
        json += "\"" + database[i] + "\"";
        if (i < database.size() - 1) json += ", ";
    }
    json += "]";

    return http::Response(http::StatusCode::Ok)
        .add_header(http::header::kContentType, http::header::ContentType::Value::ApplicationJson)
        .add_body(std::move(json));
}

static http::Response add_item_handler(const http::Request& req)
{
    if (req.body.empty()) {
        return http::Response(http::ServerErr(http::StatusCode::BadRequest, "Body cannot be empty!"));
    }

    std::lock_guard<std::mutex> lock(db_mutex);
    database.push_back(req.body);
    logger::info("Added new item: {}", req.body);

    return http::Response(http::StatusCode::Ok)
        .add_header(http::header::kContentType, http::header::ContentType::Value::TextHtml)
        .add_body("Item added successfully.");
}

int main()
{
    net::Context context;
    http::Server srv = http::ServerBuilder()
        .set_public_dir(public_dir)
        .add_route("api/items", http::RequestMethod::Get, get_items_handler)
        .add_route("api/items", http::RequestMethod::Post, add_item_handler)
        .add_route("api/items", http::RequestMethod::Delete, [&](const http::Request& req)
        {
            std::lock_guard<std::mutex> lock(db_mutex);

            size_t count = database.size();
            database.clear();

            return http::Response(http::StatusCode::Ok)
                .add_header(http::header::kContentType, http::header::ContentType::Value::TextHtml)
                .add_body(std::format("Deleted {} items.", count));
        })
        .build();
    srv.run();
}
