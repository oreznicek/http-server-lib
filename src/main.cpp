#include "http/server.hpp"

int main() {
    // TODO: allow binding to port `80` `setcap` on linux and windows???

    //int num_threads = std::thread::hardware_concurrency();
    //std::cout << num_threads << std::endl;

    http::Server server = http::ServerBuilder()
        .build();

    server.run();
}
