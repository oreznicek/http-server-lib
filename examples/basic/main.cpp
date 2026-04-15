#include <http/server.hpp>

int main()
{
    http::Server srv = http::ServerBuilder().build();
    srv.run();
}
