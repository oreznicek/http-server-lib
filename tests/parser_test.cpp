#include "test_common.hpp"
#include "response_parser.hpp"

#include <iostream>
#include <thread>
#include <chrono>

#include <http/server.hpp>

TEST_CASE(missing_host, "Missing Host Header")
{
    http::Server srv = http::ServerBuilder().build();
    net::SocketAddr4 srv_addr = net::SocketAddr4("127.0.0.1", http::kServerDefaultPort);

    std::cout << "Starting HTTP server in background thread ..." << std::endl;
    std::jthread srv_thread([&srv]() {
        srv.run();
    });

    timeval timeout = { .tv_sec = 5, .tv_usec = 0 };
    http::Connection conn(net::ClientSocket(srv_addr, &timeout));
    http::ResponseParser parser;

    std::string request = "GET / HTTP/1.1\r\n\r\n";

    std::cout << "Sending malformed request:" << std::endl;
    print_http_request(request);
    conn.send(request);

    std::cout << "Parsing response ..." << std::endl;
    http::Response res = parser.parse_response(conn);

    srv.stop();

    if (!res.valid) {
        std::cout << "Reponse isn't in valid format" << std::endl;
        return false;
    }

    std::cout << " - Expected status code: " << static_cast<int>(http::StatusCode::BadRequest) << std::endl;
    std::cout << " - Got status code:\t " << static_cast<int>(res.code) << std::endl;
    if (!res.valid || res.code != http::StatusCode::BadRequest) {
        return false;
    }

    return true;
}
