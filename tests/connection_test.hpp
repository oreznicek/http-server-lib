#include <http/connection.hpp>

#include <iostream>

inline bool test_ru(http::Connection& conn, std::string_view delimiter, std::size_t max_bytes, std::string_view expected)
{
    std::cout << "conn.read_until(\"" << delimiter << "\", " << max_bytes << ")" << std::endl;
    auto msg_recv = conn.read_until(delimiter, max_bytes); 
    if (!msg_recv.has_value()) {
        std::cout << "read_until error: " << (int)msg_recv.error() << std::endl;
        return false;
    }
    bool success = *msg_recv == expected;
    std::cout << "(expected) " << expected;
    if (success) {
        std::cout << " == ";
    } else {
        std::cout << " != ";
    }
    std::cout << *msg_recv << " (actual)" << std::endl;
    return success;
}

inline bool test_ru(http::Connection& conn, std::string_view delimiter, std::size_t max_bytes, http::StatusCode expected)
{
    std::cout << "conn.read_until(\"" << delimiter << "\", " << max_bytes << ")" << std::endl;
    auto msg_recv = conn.read_until(delimiter, max_bytes); 
    if (msg_recv.has_value()) {
        std::cout << "read_until returned: " << *msg_recv << std::endl;
        return false;
    }
    bool success = msg_recv.error() == expected;
    std::cout << "(expected) " << (int)expected;
    if (success) {
        std::cout << " == ";
    } else {
        std::cout << " != ";
    }
    std::cout << (int)msg_recv.error() << " (actual)" << std::endl;
    return success;
}

inline bool test_r(http::Connection& conn, std::size_t bytes, std::string_view expected)
{
    std::cout << "conn.read(" << bytes << ")" << std::endl;
    auto msg_recv = conn.read(bytes); 
    if (!msg_recv.has_value()) {
        std::cout << "read error: " << (int)msg_recv.error() << std::endl;
        return false;
    }
    bool success = *msg_recv == expected;
    std::cout << "(expected) " << expected;
    if (success) {
        std::cout << " == ";
    } else {
        std::cout << " != ";
    }
    std::cout << *msg_recv << " (actual)" << std::endl;
    return success;
}

inline bool test_r(http::Connection& conn, std::size_t bytes, http::StatusCode expected)
{
    std::cout << "conn.read(" << bytes << ")" << std::endl;
    auto msg_recv = conn.read(bytes); 
    if (msg_recv.has_value()) {
        std::cout << "read returned: " << *msg_recv << std::endl;
        return false;
    }
    bool success = msg_recv.error() == expected;
    std::cout << "(expected) " << (int)expected;
    if (success) {
        std::cout << " == ";
    } else {
        std::cout << " != ";
    }
    std::cout << (int)msg_recv.error() << " (actual)" << std::endl;
    return success;
}
