#include "test_common.hpp"

#include <http/server.hpp>

#include <utility>

TEST_CASE(only_ipv4, "Enable only IPv4")
{
    http::Server srv = http::ServerBuilder()
        .set_port(http::kSelectRandomPort)
        .enable_ipv4()
        .disable_ipv6()
        .build();

    timeval timeout = { .tv_sec = 0, .tv_usec = 3000 };
    bool result;

    std::cout << "Trying to connect to server with IPv4 ... ";
    try {
        std::ignore = net::ClientSocket(srv.get_addr(), &timeout);
        std::cout << "[SUCCESS]" << std::endl;
        result = true;
    } catch (std::runtime_error) {
        std::cout << "[FAILED]" << std::endl;
        result = false;
    }

    std::cout << "Trying to connect to server with IPv6 ... ";
    try {
        std::ignore = net::ClientSocket(srv.get_addr6(), &timeout);
        std::cout << "[SUCCESS]" << std::endl;
        return false;
    } catch (std::runtime_error) {
        std::cout << "[FAILED]" << std::endl;
        return result;
    }
}

TEST_CASE(only_ipv6, "Enable only IPv6")
{
    http::Server srv = http::ServerBuilder()
        .set_port(http::kSelectRandomPort)
        .enable_ipv6()
        .disable_ipv4()
        .build();

    timeval timeout = { .tv_sec = 0, .tv_usec = 3000 };
    bool result;

    std::cout << "Trying to connect to server with IPv4 ... ";
    try {
        std::ignore = net::ClientSocket(srv.get_addr(), &timeout);
        std::cout << "[SUCCESS]" << std::endl;
        result = false;
    } catch (std::runtime_error) {
        std::cout << "[FAILED]" << std::endl;
        result = true;
    }

    std::cout << "Trying to connect to server with IPv6 ... ";
    try {
        std::ignore = net::ClientSocket(srv.get_addr6(), &timeout);
        std::cout << "[SUCCESS]" << std::endl;
        return result;
    } catch (std::runtime_error) {
        std::cout << "[FAILED]" << std::endl;
        return false;
    }
}

TEST_CASE(ip_disabled, "Both IPv4 and IPv6 are disabled")
{
    std::cout << "Check that ServerBuilder throws exception ... " << std::endl;
    try {
        std::ignore = http::ServerBuilder()
            .set_port(http::kSelectRandomPort)
            .disable_ipv4()
            .disable_ipv6()
            .build();
        return false;
    } catch (std::runtime_error) {
        return true;
    }
}
