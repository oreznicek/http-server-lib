#ifndef _TEST_HELPERS_HPP
#define _TEST_HELPERS_HPP

#include <http/response.hpp>
#include <http/server.hpp>

#include <string>
#include <iostream>

http::Response send_request_get_response(http::Server& srv, const std::string& request);

template<typename T>
bool test_eq(const T& value, const T& expected)
{
    bool success = value == expected;
    std::cout << " -> (expected) " << expected;
    if (success) {
        std::cout << " == ";
    } else {
        std::cout << " != ";
    }
    std::cout << value << " (actual)" << std::endl;
    return success;
}

template<typename T>
bool test_diff(const T& value, const T& diff)
{
    bool success = value != diff;
    std::cout << " -> (diff) " << diff;
    if (success) {
        std::cout << " != ";
    } else {
        std::cout << " == ";
    }
    std::cout << value << " (actual)" << std::endl;
    return success;
}

bool test_status_code_eq(http::Server& srv, const std::string& request, http::StatusCode expected);
bool test_status_code_diff(http::Server& srv, const std::string& request, http::StatusCode diff);

std::string read_file(const std::filesystem::path& file_path);

bool test_response_eq(http::Server& srv, const std::string& request, const http::Response& expected);

#endif
