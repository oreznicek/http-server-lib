#include "test_common.hpp"
#include "connection_test.hpp"

TEST_CASE(ru_simple)
{
    std::size_t max_bytes = 10;
    auto [dial_sock, answer_sock] = net::ClientSocket::create_socketpair();
    http::Connection answer_conn(std::move(answer_sock));

    std::string message = "who does#this?";
    dial_sock.send(message);
    std::cout << "Client sends message: '" << message << "'" << std::endl;

    bool result = test_ru(answer_conn, " ", max_bytes, "who");
    result &= test_ru(answer_conn, "#", max_bytes, "does");
    result &= test_ru(answer_conn, "?", max_bytes, "this");

    return result;
}

TEST_CASE(ru_longer_delim)
{
    std::size_t max_bytes = 10;
    auto [dial_sock, answer_sock] = net::ClientSocket::create_socketpair();
    http::Connection answer_conn(std::move(answer_sock));

    std::string message = "who$$$does@%this?";
    dial_sock.send(message);
    std::cout << "Client sends message: '" << message << "'" << std::endl;

    bool result = test_ru(answer_conn, "$$$", max_bytes, "who");
    result &= test_ru(answer_conn, "@%", max_bytes, "does");
    result &= test_ru(answer_conn, "?", max_bytes, "this");

    return result;
}

TEST_CASE(ru_timeout)
{
    std::size_t max_bytes = 10;
    auto [dial_sock, answer_sock] = net::ClientSocket::create_socketpair();
    http::Connection answer_conn(std::move(answer_sock));

    std::string message = "who $ $$";
    dial_sock.send(message);
    std::cout << "Client sends message: '" << message << "'" << std::endl;

    return test_ru(answer_conn, "$$$", max_bytes, http::StatusCode::RequestTimeout);
}

TEST_CASE(ru_max_bytes)
{
    auto [dial_sock, answer_sock] = net::ClientSocket::create_socketpair();
    http::Connection answer_conn(std::move(answer_sock));

    std::string message1 = "12345$";
    dial_sock.send(message1);
    std::cout << "Client sends message: '" << message1 << "'" << std::endl;

    bool result = test_ru(answer_conn, "$", 4, http::StatusCode::ContentTooLarge);

    std::string message2 = "1234$%";
    dial_sock.send(message2);
    std::cout << "Client sends message: '" << message2 << "'" << std::endl;

    result &= test_ru(answer_conn, "$%", 4, "1234");

    return result;
}

TEST_CASE(ru_null_byte)
{
    auto [dial_sock, answer_sock] = net::ClientSocket::create_socketpair();
    http::Connection answer_conn(std::move(answer_sock));

    using namespace std::string_literals;

    std::string message = "12\0004$"s;
    dial_sock.send(message);
    std::cout << "Client sends message: '" << message << "'" << std::endl;

    bool result = test_ru(answer_conn, "$", 4, "12\0004"s);

    dial_sock.send(message);
    std::cout << "Client sends message: '" << message << "'" << std::endl;

    result &= !test_ru(answer_conn, "$", 4, "124"s);

    return result;
}

TEST_CASE(r_simple)
{
    auto [dial_sock, answer_sock] = net::ClientSocket::create_socketpair();
    http::Connection answer_conn(std::move(answer_sock));

    std::string message = "123**6789";
    dial_sock.send(message);
    std::cout << "Client sends message: '" << message << "'" << std::endl;

    bool result = test_r(answer_conn, 3, "123");
    result &= test_r(answer_conn, 2, "**");
    result &= test_r(answer_conn, 4, "6789");

    return result;
}

TEST_CASE(r_timeout)
{
    auto [dial_sock, answer_sock] = net::ClientSocket::create_socketpair();
    http::Connection answer_conn(std::move(answer_sock));

    std::string message = "msg";
    dial_sock.send(message);
    std::cout << "Client sends message: '" << message << "'" << std::endl;

    return test_r(answer_conn, 4, http::StatusCode::RequestTimeout);

}
