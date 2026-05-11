#include "http/server.hpp"
#include "test_common.hpp"
#include "test_helpers.hpp"

#include <array>
#include <format>

TEST_CASE(unknown_method, "Unknown Request Method")
{
    http::Server srv = http::ServerBuilder()
        .set_port(http::kSelectRandomPort)
        .build();

    std::array<std::string, 2> requests = {
        std::format(
            "GET-NEW / {}\r\n"
            "Host: www.example.com\r\n\r\n",
            http::kServerHttpVersion
        ),

        std::format(
            ":] / {}\r\n"
            "Host: www.example.com\r\n\r\n",
            http::kServerHttpVersion
        )
    };

    http::StatusCode expected = http::StatusCode::NotImplemented;
    bool result = true;

    for (const auto& req : requests) {
        result &= test_status_code_eq(srv, req, expected);
    }

    return result;
}

TEST_CASE(missing_host, "Missing Host Header")
{
    http::Server srv = http::ServerBuilder()
        .set_port(http::kSelectRandomPort)
        .build();

    std::array<std::string, 2> requests = {
        std::format(
            "{} / {}\r\n\r\n",
            http::method::kGet,
            http::kServerHttpVersion
        ),

        std::format(
            "{} / {}\r\n"
            "???: www.example.com\r\n\r\n",
            http::method::kGet,
            http::kServerHttpVersion
        )
    };

    http::StatusCode expected = http::StatusCode::BadRequest;
    bool result = true;

    for (const auto& req : requests) {
        result &= test_status_code_eq(srv, req, expected);
    }

    return result;
}

TEST_CASE(invalid_version, "Invalid HTTP version")
{
    http::Server srv = http::ServerBuilder()
        .set_port(http::kSelectRandomPort)
        .build();

    std::array<std::string, 2> requests = {
        std::format(
            "{} / HTTP/2.0\r\n"
            "Host: www.example.com\r\n\r\n",
            http::method::kGet
        ),

        std::format(
            "{} / FJKDEF\r\n"
            "Host: www.example.com\r\n\r\n",
            http::method::kGet
        )
    };

    http::StatusCode expected = http::StatusCode::HttpVersionNotSupported;
    bool result = true;

    for (const auto& req : requests) {
        result &= test_status_code_eq(srv, req, expected);
    }

    return result;
}

TEST_CASE(long_uri, "Uri Too Long")
{
    http::Server srv1 = http::ServerBuilder()
        .set_port(http::kSelectRandomPort)
        .set_request_target_size_limit(7)
        .build();

    std::string err_request1 = std::format(
        "{} /longest {}\r\n"
        "Host: www.example.com\r\n\r\n",
        http::method::kGet,
        http::kServerHttpVersion
    );
    std::string ok_request1 = std::format(
        "{} /longes {}\r\n"
        "Host: www.example.com\r\n\r\n",
        http::method::kGet,
        http::kServerHttpVersion
    );

    bool result = test_status_code_eq(srv1, err_request1, http::StatusCode::UriTooLong);
    result &= test_status_code_diff(srv1, ok_request1, http::StatusCode::UriTooLong);

    http::Server srv2 = http::ServerBuilder()
        .set_port(http::kSelectRandomPort)
        .set_request_target_size_limit(3)
        .build();

    std::string err_request2 = std::format(
        "{} /foo {}\r\n"
        "Host: www.example.com\r\n\r\n",
        http::method::kGet,
        http::kServerHttpVersion
    );
    std::string ok_request2 = std::format(
        "{} /fo {}\r\n"
        "Host: www.example.com\r\n\r\n",
        http::method::kGet,
        http::kServerHttpVersion
    );

    result &= test_status_code_eq(srv2, err_request2, http::StatusCode::UriTooLong);
    result &= test_status_code_diff(srv2, ok_request2, http::StatusCode::UriTooLong);

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
    std::string request = std::format(
        "{} / {}\r\n"
        "Host: www.example.com\r\n"
        "Invalid header\r\n\r\n",
        http::method::kGet,
        http::kServerHttpVersion
    );
    return test_status_code_eq(srv, request, http::StatusCode::BadRequest);
}

TEST_CASE(missing_key, "Missing Header Key")
{
    http::Server srv = http::ServerBuilder()
        .set_port(http::kSelectRandomPort)
        .build();
    std::string request = std::format(
        "{} / {}\r\n"
        "Host: www.example.com\r\n"
        ": only value\r\n\r\n",
        http::method::kGet,
        http::kServerHttpVersion
    );
    return test_status_code_eq(srv, request, http::StatusCode::BadRequest);
}

TEST_CASE(missing_key_value, "Missing Header Key and Value")
{
    http::Server srv = http::ServerBuilder()
        .set_port(http::kSelectRandomPort)
        .build();
    std::string request = std::format(
        "{} / {}\r\n"
        "Host: www.example.com\r\n"
        ":\r\n\r\n",
        http::method::kGet,
        http::kServerHttpVersion
    );
    return test_status_code_eq(srv, request, http::StatusCode::BadRequest);
}

TEST_CASE(insensitive_key, "Case Insensitive Header Key")
{
    http::Server srv = http::ServerBuilder()
        .set_port(http::kSelectRandomPort)
        .build();

    std::string request1 = std::format(
        "{} / {}\r\n"
        "host: www.example.com\r\n\r\n",
        http::method::kGet,
        http::kServerHttpVersion
    );
    std::string request2 = std::format(
        "{} / {}\r\n"
        "hOsT: www.google.com\r\n\r\n",
        http::method::kGet,
        http::kServerHttpVersion
    );

    bool result = test_status_code_diff(srv, request1, http::StatusCode::BadRequest);
    result &= test_status_code_diff(srv, request2, http::StatusCode::BadRequest);

    return result;
}
