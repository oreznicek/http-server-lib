#include "test_helpers.hpp"
#include "response_parser.hpp"

#include <iostream>
#include <sstream>
#include <thread>

#include <http/server.hpp>

void print_http_request(const std::string& str)
{
    std::istringstream stream(str);
    std::string line;
    //std::cout << "---------- [ HTTP REQUEST START ] ----------" << std::endl;
    while (std::getline(stream, line)) {
        std::cout << ">> " << line << std::endl;
    }
    //std::cout << "---------- [  HTTP REQUEST END  ] ----------" << std::endl;
}

http::Response send_request_get_response(http::Server& srv, const std::string& request)
{
    std::jthread srv_thread([&srv]() {
        srv.run();
    });

    timeval timeout = { .tv_sec = 0, .tv_usec = 3000 };
    http::Connection conn(net::ClientSocket(srv.get_addr(), &timeout));
    http::ResponseParser parser;

    std::cout << "Sending request:" << std::endl;
    print_http_request(request);
    conn.send(request);

    std::cout << "Parsing response ..." << std::endl;
    http::Response res = parser.parse_response(conn);

    srv.stop();
    return res;
}

bool test_status_code_eq(http::Server& srv, const std::string& request, http::StatusCode expected)
{
    http::Response res = send_request_get_response(srv, request);

    if (!res.valid) {
        std::cout << "Response is invalid" << std::endl;
        return false;
    }

    bool success = res.code == expected;

    std::cout << " -> (expected) " << static_cast<int>(expected);
    if (success) {
        std::cout << " == ";
    } else {
        std::cout << " != ";
    }
    std::cout << static_cast<int>(res.code) << " (actual)" << std::endl;

    return success;
}

bool test_status_code_diff(http::Server& srv, const std::string& request, http::StatusCode diff)
{
    http::Response res = send_request_get_response(srv, request);

    if (!res.valid) {
        std::cout << "Response is invalid" << std::endl;
        return false;
    }

    bool success = res.code != diff;

    std::cout << " -> (different) " << static_cast<int>(diff);
    if (success) {
        std::cout << " != ";
    } else {
        std::cout << " = ";
    }
    std::cout << static_cast<int>(res.code) << " (actual)" << std::endl;

    return success;
}
