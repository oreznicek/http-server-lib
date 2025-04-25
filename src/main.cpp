#include "HttpServer.hpp"

#define PORT 8080

int main() {
    // TODO: allow binding to port `80` `setcap` on linux and windows???

    //int num_threads = std::thread::hardware_concurrency();
    //std::cout << num_threads << std::endl;

    HttpServer server = HttpServerBuilder()
        .build();

    server.run();
}
