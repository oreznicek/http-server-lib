#include <http/server.hpp>

int main()
{
    net::Context context;
    http::Server srv = http::ServerBuilder().build();
    srv.run();
}
