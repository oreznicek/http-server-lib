#include "test_common.hpp"
#include "test_helpers.hpp"

#include <http/server.hpp>

#include <format>
#include <utility>

/* ServerBuilder methods which are not tested in this file and why:
 *  - set_public_dir()          ... not implemented in the Server
 *  - set_request_target_size() ... tested in "Uri Too Long" in parser_test
 *  - set_request_timeout()     ... tested in empty_request in parser_test
 */

TEST_CASE(set_port, "Running server on specific port")
{
    uint16_t port = 8081;
    bool result;

    {
        std::cout << "Create new HTTP server and bind it to port " << port << std::endl;
        http::Server srv = http::ServerBuilder()
            .set_port(port)
            .build();

        std::cout << "Trying to connect to server on port " << port << " ... ";
        try {
            std::ignore = net::ClientSocket(net::SocketAddr4("127.0.0.1", port));
            std::cout << "[SUCCESS]" << std::endl;
            result = true;
        } catch (std::runtime_error) {
            std::cout << "[FAILED]" << std::endl;
            result = false;
        }
    }
    std::cout << "Server was destroyed, so the port shouldn't be bound" << std::endl;

    std::cout << "Trying to connect to server on port " << port << " ... ";
    try {
        std::ignore = net::ClientSocket(net::SocketAddr4("127.0.0.1", port));
        std::cout << "[SUCCESS]" << std::endl;
        return false;
    } catch (std::runtime_error) {
        std::cout << "[FAILED]" << std::endl;
        return result;
    }
}

TEST_CASE(only_ipv4, "Enable only IPv4")
{
    http::Server srv = http::ServerBuilder()
        .set_port(http::kSelectRandomPort)
        .enable_ipv4()
        .disable_ipv6()
        .build();

    bool result;

    std::cout << "Trying to connect to server with IPv4 ... ";
    try {
        std::ignore = net::ClientSocket(srv.get_addr());
        std::cout << "[SUCCESS]" << std::endl;
        result = true;
    } catch (std::runtime_error) {
        std::cout << "[FAILED]" << std::endl;
        result = false;
    }

    std::cout << "Trying to connect to server with IPv6 ... ";
    try {
        std::ignore = net::ClientSocket(srv.get_addr6());
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

    bool result;

    std::cout << "Trying to connect to server with IPv4 ... ";
    try {
        std::ignore = net::ClientSocket(srv.get_addr());
        std::cout << "[SUCCESS]" << std::endl;
        result = false;
    } catch (std::runtime_error) {
        std::cout << "[FAILED]" << std::endl;
        result = true;
    }

    std::cout << "Trying to connect to server with IPv6 ... ";
    try {
        std::ignore = net::ClientSocket(srv.get_addr6());
        std::cout << "[SUCCESS]" << std::endl;
        return result;
    } catch (std::runtime_error) {
        std::cout << "[FAILED]" << std::endl;
        return false;
    }
}

TEST_CASE(both_ip_enabled, "Both IPv4 and IPv6 are enabled")
{
    http::Server srv = http::ServerBuilder()
        .set_port(http::kSelectRandomPort)
        .enable_ipv4()
        .enable_ipv6()
        .build();

    timeval timeout = { .tv_sec = 0, .tv_usec = 3000 };
    bool result;

    std::cout << "Trying to connect to server with IPv4 ... ";
    try {
        std::ignore = net::ClientSocket(srv.get_addr());
        std::cout << "[SUCCESS]" << std::endl;
        result = true;
    } catch (std::runtime_error) {
        std::cout << "[FAILED]" << std::endl;
        result = false;
    }

    std::cout << "Trying to connect to server with IPv6 ... ";
    try {
        std::ignore = net::ClientSocket(srv.get_addr6());
        std::cout << "[SUCCESS]" << std::endl;
        return result;
    } catch (std::runtime_error) {
        std::cout << "[FAILED]" << std::endl;
        return false;
    }
}

TEST_CASE(both_ip_disabled, "Both IPv4 and IPv6 are disabled")
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

TEST_CASE(headers_size_limit)
{
    std::string ok_headers = std::format(
        "{} / {}\r\n"
        "Host: www.example.com",
        http::method::kGet,
        http::kServerHttpVersion
    );

    http::Server srv = http::ServerBuilder()
        .set_port(http::kSelectRandomPort)
        .set_request_headers_size_limit(ok_headers.size())
        .build();

    http::StatusCode err_code = http::StatusCode::RequestHeaderFieldsTooLarge;
    std::string ok_request = ok_headers + "\r\n\r\n";
    std::string err_request = std::format(
        "{} / {}\r\n"
        "Host: www.example1.com\r\n\r\n",
        http::method::kGet,
        http::kServerHttpVersion
    );

    bool result = test_status_code_diff(srv, ok_request, err_code);
    result = result & test_status_code_eq(srv, err_request, err_code);

    return result;
}

TEST_CASE(body_size_limit)
{
    std::string ok_body = "{\"username\": \"test_user\"}";

    http::Server srv = http::ServerBuilder()
        .set_port(http::kSelectRandomPort)
        .set_request_body_size_limit(ok_body.size())
        .build();

    http::StatusCode err_code = http::StatusCode::ContentTooLarge;
    std::string ok_request = std::format(
        "{} /api/users {}\r\n"
        "Host: localhost:8080\r\n"
        "Content-Type: application/json\r\n"
        "Content-Length: {}\r\n\r\n"
        "{}",
        http::method::kPost,
        http::kServerHttpVersion,
        ok_body.size(),
        ok_body
    );
    std::string err_body = "{\"username\": \"test_user1\"}";
    std::string err_request = std::format(
        "{} /api/users {}\r\n"
        "Host: localhost:8080\r\n"
        "Content-Type: application/json\r\n"
        "Content-Length: {}\r\n\r\n"
        "{}",
        http::method::kPost,
        http::kServerHttpVersion,
        err_body.size(),
        err_body
    );

    bool result = test_status_code_diff(srv, ok_request, err_code);
    result = result & test_status_code_eq(srv, err_request, err_code);

    return result;
}
