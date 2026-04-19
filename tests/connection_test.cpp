#include "test_common.hpp"

#include <http/connection.hpp>

#include <cassert>

/*
 */

TEST_CASE(read_until_simple)
{
    std::size_t max_bytes = 10;
    auto [dial_sock, answer_sock] = net::ClientSocket::create_socketpair();
    http::Connection answer_conn(std::move(answer_sock));

    std::string message = "who does#this?";

    dial_sock.send(message);
    auto result = answer_conn.read_until(" ", max_bytes); 
    if (!result.has_value()) {
        std::cout << "read_until error: " << (int)result.error() << std::endl;
        return false;
    }
    std::cout << "(expected) who == " << *result << " (actual)" << std::endl;
    assert(*result == "who");

    result = answer_conn.read_until("#", max_bytes); 
    if (!result.has_value()) {
        std::cout << "read_until error: " << (int)result.error() << std::endl;
        return false;
    }
    assert(*result == "does");

    result = answer_conn.read_until("?", max_bytes); 
    if (!result.has_value()) {
        std::cout << "read_until error: " << (int)result.error() << std::endl;
        return false;
    }
    assert(*result == "this");

    return true;
}
