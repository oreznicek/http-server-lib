#include "test_helpers.hpp"
#include "response_parser.hpp"

#include <iostream>
#include <sstream>
#include <thread>

#include <http/server.hpp>

void print_lines(const std::string& str)
{
    std::istringstream stream(str);
    std::string line;
    while (std::getline(stream, line)) {
        std::cout << ">> " << line << std::endl;
    }
}

http::Response send_request_get_response(http::Server& srv, const std::string& request)
{
    std::jthread srv_thread([&srv]() {
        srv.run();
    });

    http::Connection conn(net::ClientSocket(srv.get_addr()));
    http::ResponseParser parser;

    std::cout << "Sending request:" << std::endl;
    print_lines(request);
    conn.send(request);

    std::cout << "Parsing response ..." << std::endl;
    auto res = parser.parse_response(conn);
    if (!res.has_value()) {
        std::cerr << res.error() << std::endl;
        return http::Response();
    }

    //print_lines(res->to_string());

    srv.stop();
    return *res;
}

bool test_status_code_eq(http::Server& srv, const std::string& request, http::StatusCode expected)
{
    http::Response res = send_request_get_response(srv, request);
    if (res.code == http::StatusCode::None) {
        return false;
    }
    return test_eq(static_cast<int>(res.code), static_cast<int>(expected));
}

bool test_status_code_diff(http::Server& srv, const std::string& request, http::StatusCode diff)
{
    http::Response res = send_request_get_response(srv, request);
    if (res.code == http::StatusCode::None) {
        return false;
    }
    return test_diff(static_cast<int>(res.code), static_cast<int>(diff));
}

bool test_response_eq(http::Server& srv, const std::string& request, const http::Response& expected)
{
    http::Response res = send_request_get_response(srv, request);

    bool result = true;

    print_lines(res.to_string());

    std::cout << "Comparing status code ..." << std::endl;
    result &= test_eq((int)res.code, (int)expected.code);

    std::cout << "Comparing headers ..." << std::endl;
    for (const auto& [key, value] : expected.headers) {
        if (auto it = res.headers.find(key); it != res.headers.end()) {
            std::cout << key << " ";
            result &= test_eq(it->second, value);
        } else {
            std::cout << " -> Header '" << key << "' is missing.\n";
            result = false;
        }
    }

    return result;
}
