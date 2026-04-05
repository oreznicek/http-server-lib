#include "test_common.hpp"
#include "test_helpers.hpp"

#include <array>

TEST_CASE(unknown_method, "Unknown Request Method")
{
    http::Server srv = http::ServerBuilder()
        .set_port(http::kSelectRandomPort)
        .build();

    std::array<std::string, 2> requests = {
        "GET-NEW / HTTP/1.1\r\n"
        "Host: www.example.com\r\n\r\n",

        ":] / HTTP/1.1\r\n"
        "Host: www.example.com\r\n\r\n",
    };

    http::StatusCode expected = http::StatusCode::NotImplemented;
    bool result = true;

    for (const auto& req : requests) {
        result = result && test_status_code_eq(srv, req, expected);
    }

    return result;
}

TEST_CASE(missing_host, "Missing Host Header")
{
    http::Server srv = http::ServerBuilder()
        .set_port(http::kSelectRandomPort)
        .build();

    std::array<std::string, 2> requests = {
        "GET / HTTP/1.1\r\n\r\n",

        "GET / HTTP/1.1\r\n"
        "???: www.example.com\r\n\r\n"
    };

    http::StatusCode expected = http::StatusCode::BadRequest;
    bool result = true;

    for (const auto& req : requests) {
        result = result && test_status_code_eq(srv, req, expected);
    }

    return result;
}

TEST_CASE(invalid_version, "Invalid HTTP version")
{
    http::Server srv = http::ServerBuilder()
        .set_port(http::kSelectRandomPort)
        .build();

    std::array<std::string, 2> requests = {
        "GET / HTTP/2.0\r\n"
        "Host: www.example.com\r\n\r\n",

        "GET / FJKDEF\r\n"
        "Host: www.example.com\r\n\r\n"
    };

    http::StatusCode expected = http::StatusCode::HttpVersionNotSupported;
    bool result = true;

    for (const auto& req : requests) {
        result = result && test_status_code_eq(srv, req, expected);
    }

    return result;
}

TEST_CASE(long_uri, "Uri Too Long")
{
    http::Server srv1 = http::ServerBuilder()
        .set_port(http::kSelectRandomPort)
        .set_request_target_size_limit(7)
        .build();

    std::string err_request1 =
        "GET /longest HTTP/1.1\r\n"
        "Host: www.example.com\r\n\r\n";
    std::string ok_request1 =
        "GET /longes HTTP/1.1\r\n"
        "Host: www.example.com\r\n\r\n";

    bool result = test_status_code_eq(srv1, err_request1, http::StatusCode::UriTooLong);
    result = result && test_status_code_diff(srv1, ok_request1, http::StatusCode::UriTooLong);

    http::Server srv2 = http::ServerBuilder()
        .set_port(http::kSelectRandomPort)
        .set_request_target_size_limit(3)
        .build();

    std::string err_request2 =
        "GET /foo HTTP/1.1\r\n"
        "Host: www.example.com\r\n\r\n";
    std::string ok_request2 =
        "GET /fo HTTP/1.1\r\n"
        "Host: www.example.com\r\n\r\n";

    result = result && test_status_code_eq(srv2, err_request2, http::StatusCode::UriTooLong);
    result = result && test_status_code_diff(srv2, ok_request2, http::StatusCode::UriTooLong);

    return result;
}

TEST_CASE(empty_request)
{
    timeval timeout = { .tv_sec = 0, .tv_usec = 1000 };
    http::Server srv = http::ServerBuilder()
        .set_port(http::kSelectRandomPort)
        .set_request_timeout(timeout)
        .build();

    std::string request = "";
    return test_status_code_eq(srv, request, http::StatusCode::RequestTimeout);
}

TEST_CASE(missing_colon, "Missing Header Colon")
{
    http::Server srv = http::ServerBuilder()
        .set_port(http::kSelectRandomPort)
        .build();
    std::string request =
        "GET / HTTP/1.1\r\n"
        "Host: www.example.com\r\n"
        "Invalid header\r\n\r\n";
    return test_status_code_eq(srv, request, http::StatusCode::BadRequest);
}

TEST_CASE(missing_key, "Missing Header Key")
{
    http::Server srv = http::ServerBuilder()
        .set_port(http::kSelectRandomPort)
        .build();
    std::string request =
        "GET / HTTP/1.1\r\n"
        "Host: www.example.com\r\n"
        ": only value\r\n\r\n";
    return test_status_code_eq(srv, request, http::StatusCode::BadRequest);
}

TEST_CASE(missing_key_value, "Missing Header Key and Value")
{
    http::Server srv = http::ServerBuilder()
        .set_port(http::kSelectRandomPort)
        .build();
    std::string request =
        "GET / HTTP/1.1\r\n"
        "Host: www.example.com\r\n"
        ":\r\n\r\n";
    return test_status_code_eq(srv, request, http::StatusCode::BadRequest);
}
