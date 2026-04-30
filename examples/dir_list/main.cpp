#include <http/server.hpp>

#include <source_location>
#include <filesystem>

namespace fs = std::filesystem;

static fs::path public_dir = fs::path(std::source_location::current().file_name()).remove_filename() / "public";

int main()
{
    net::Context context;
    http::Server srv = http::ServerBuilder()
        .set_public_dir(public_dir)
        .enable_directory_listing()
        .build();

    srv.run();
}
