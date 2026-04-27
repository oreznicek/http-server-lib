#include "test_common.hpp"
#include "test_helpers.hpp"

#include <http/http.hpp>

TEST_CASE(empty)
{
    std::string target = "";
    std::cout << "Raw request target: '" << target << "'" << std::endl;

    auto req_target = http::RequestTarget::from(target);
    if (!req_target.has_value()) {
        std::cout << " -> " << req_target.error() << std::endl;
        return true;
    }

    std::cout << " -> '" << req_target->relative_path << "'" << std::endl;
    return false;
}

TEST_CASE(makes_relative_from_absolute_path)
{
    std::string target = "/api/create-user";
    std::string expected = "api/create-user";

    std::cout << "Raw request target: '" << target << "'" << std::endl;

    auto req_target = http::RequestTarget::from(target);
    if (!req_target.has_value()) {
        std::cout << " -> " << req_target.error() << std::endl;
        return false;
    }

    return test_eq(req_target->relative_path, expected);
}

TEST_CASE(empty_parts)
{
    std::string target = "/dir1//dir2///file.txt";
    std::string expected = "dir1/dir2/file.txt";

    std::cout << "Raw request target: '" << target << "'" << std::endl;

    auto req_target = http::RequestTarget::from(target);
    if (!req_target.has_value()) {
        std::cout << " -> " << req_target.error() << std::endl;
        return false;
    }

    return test_eq(req_target->relative_path, expected);
}

TEST_CASE(single_dot)
{
    std::string target = "dir1/./dir2/././file.txt";
    std::string expected = "dir1/dir2/file.txt";

    std::cout << "Raw request target: '" << target << "'" << std::endl;

    auto req_target = http::RequestTarget::from(target);
    if (!req_target.has_value()) {
        std::cout << " -> " << req_target.error() << std::endl;
        return false;
    }

    return test_eq(req_target->relative_path, expected);
}

TEST_CASE(double_dot1)
{
    std::string target = "/dir1/./dir2/.././file.txt";
    std::string expected = "dir1/file.txt";

    std::cout << "Raw request target: '" << target << "'" << std::endl;

    auto req_target = http::RequestTarget::from(target);
    if (!req_target.has_value()) {
        std::cout << " -> " << req_target.error() << std::endl;
        return false;
    }

    return test_eq(req_target->relative_path, expected);
}

TEST_CASE(double_dot2)
{
    std::string target = "dir1/./dir2/.././dir3/dir4/../file.txt";
    std::string expected = "dir1/dir3/file.txt";

    std::cout << "Raw request target: '" << target << "'" << std::endl;

    auto req_target = http::RequestTarget::from(target);
    if (!req_target.has_value()) {
        std::cout << " -> " << req_target.error() << std::endl;
        return false;
    }

    return test_eq(req_target->relative_path, expected);
}

TEST_CASE(missing_double_slash)
{
    std::string target = "http:www.example.com/api/create-user";

    std::cout << "Raw request target: '" << target << "'" << std::endl;

    auto req_target = http::RequestTarget::from(target);
    if (!req_target.has_value()) {
        std::cout << " -> " << req_target.error() << std::endl;
        return true;
    }

    return false;
}

TEST_CASE(missing_authority)
{
    std::string target = "http:///api/create-user";

    std::cout << "Raw request target: '" << target << "'" << std::endl;

    auto req_target = http::RequestTarget::from(target);
    if (!req_target.has_value()) {
        std::cout << " -> " << req_target.error() << std::endl;
        return true;
    }

    return false;
}

TEST_CASE(absolute_form_without_path)
{
    std::string target = "http://www.example.com";
    std::string expected = "";

    auto req_target = http::RequestTarget::from(target);
    if (!req_target.has_value()) {
        std::cout << " -> " << req_target.error() << std::endl;
        return false;
    }

    return test_eq(req_target->relative_path, expected);
}

TEST_CASE(empty_parts_abs)
{
    std::string target = "http://www.google.com/dir1//dir2///file.txt";
    std::string expected = "dir1/dir2/file.txt";

    std::cout << "Raw request target: '" << target << "'" << std::endl;

    auto req_target = http::RequestTarget::from(target);
    if (!req_target.has_value()) {
        std::cout << " -> " << req_target.error() << std::endl;
        return false;
    }

    return test_eq(req_target->relative_path, expected);
}

TEST_CASE(single_dot_abs)
{
    std::string target = "http://www.example.cz/dir1/./dir2/././file.txt";
    std::string expected = "dir1/dir2/file.txt";

    std::cout << "Raw request target: '" << target << "'" << std::endl;

    auto req_target = http::RequestTarget::from(target);
    if (!req_target.has_value()) {
        std::cout << " -> " << req_target.error() << std::endl;
        return false;
    }

    return test_eq(req_target->relative_path, expected);
}

TEST_CASE(double_dot1_abs)
{
    std::string target = "http://www.mff.cuni.cz/dir1/./dir2/.././file.txt";
    std::string expected = "dir1/file.txt";

    std::cout << "Raw request target: '" << target << "'" << std::endl;

    auto req_target = http::RequestTarget::from(target);
    if (!req_target.has_value()) {
        std::cout << " -> " << req_target.error() << std::endl;
        return false;
    }

    return test_eq(req_target->relative_path, expected);
}

TEST_CASE(double_dot2_abs)
{
    std::string target = "http://www.domain-name.com/dir1/./dir2/.././dir3/dir4/../file.txt";
    std::string expected = "dir1/dir3/file.txt";

    std::cout << "Raw request target: '" << target << "'" << std::endl;

    auto req_target = http::RequestTarget::from(target);
    if (!req_target.has_value()) {
        std::cout << " -> " << req_target.error() << std::endl;
        return false;
    }

    return test_eq(req_target->relative_path, expected);
}
