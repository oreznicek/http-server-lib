#include "test_common.hpp"
#include "test_helpers.hpp"

#include <handlers/router.hpp>

TEST_CASE(empty)
{
    std::string path = "";
    std::cout << "Normalizing path: '" << path << "'" << std::endl;
    std::string expected = "/";
    return test_eq(handlers::normalize_uri(path), expected);
}

TEST_CASE(empty_parts)
{
    std::string path = "dir1//dir2///file.txt";
    std::cout << "Normalizing path: '" << path << "'" << std::endl;
    std::string expected = "/dir1/dir2/file.txt";
    return test_eq(handlers::normalize_uri(path), expected);
}

TEST_CASE(single_dot)
{
    std::string path = "dir1/./dir2/././file.txt";
    std::cout << "Normalizing path: '" << path << "'" << std::endl;
    std::string expected = "/dir1/dir2/file.txt";
    return test_eq(handlers::normalize_uri(path), expected);
}

TEST_CASE(double_dot1)
{
    std::string path = "dir1/./dir2/.././file.txt";
    std::cout << "Normalizing path: '" << path << "'" << std::endl;
    std::string expected = "/dir1/file.txt";
    return test_eq(handlers::normalize_uri(path), expected);
}

TEST_CASE(double_dot2)
{
    std::string path = "dir1/./dir2/.././dir3/dir4/../file.txt";
    std::cout << "Normalizing path: '" << path << "'" << std::endl;
    std::string expected = "/dir1/dir3/file.txt";
    return test_eq(handlers::normalize_uri(path), expected);
}
