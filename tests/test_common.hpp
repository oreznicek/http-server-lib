#ifndef _TEST_COMMON_HPP
#define _TEST_COMMON_HPP

#include <iostream>
#include <vector>
#include <sstream>

inline void print_http_request(const std::string& str)
{
    std::istringstream stream(str);
    std::string line;
    std::cout << "========== [ HTTP REQUEST START ] ==========" << std::endl;
    while (std::getline(stream, line)) {
        std::cout << ">> " << line << std::endl;
    }
    std::cout << "========== [  HTTP REQUEST END  ] ==========" << std::endl;
}

using test_func_t = bool(*)();

// This holds the list of all registered tests
class TestRegistry {
private:
    std::vector<test_func_t> tests;
public:
    // Meyers' Singleton
    static TestRegistry& instance() {
        static TestRegistry inst;
        return inst;
    }

    // Add a test to the list
    void add(test_func_t func) {
        tests.push_back(func);
    }

    // Run all tests and return the CTest exit code
    int run_all() {
        int failed = 0;
        for (const test_func_t& test : tests) {
            // Call the function pointer
            if (!test()) {
                failed++;
            }
        }

        std::cout << "\n==============================\n";
        if (failed == 0) {
            std::cout << "SUCCESS: All " << tests.size() << " tests passed!\n";
            return 0;
        } else {
            std::cerr << "FAILURE: " << failed << " out of " << tests.size() << " tests failed.\n";
            return 1;
        }
    }
};

// A helper whose only job is to call `add` in its constructor
struct TestRegistrar {
    TestRegistrar(test_func_t func) {
        TestRegistry::instance().add(func);
    }
};

// Defines a test case
//  - has to be followed by a function body
#define TEST_CASE(name, ...) \
    static bool name ## _inner(); \
    static bool name() { \
        std::cout << "Test case \"" #name "\"" __VA_OPT__(<< " (" <<) __VA_ARGS__ __VA_OPT__(<< ")") << ':' << std::endl; \
        try { \
            if (!name ## _inner()) { \
                std::cout << "Test case \"" #name "\" failed" << std::endl; \
                return false; \
            } \
            std::cout << "Test case \"" #name "\" passed" << std::endl; \
            return true; \
        } catch (const std::exception &e) { \
            std::cout << "Test case \"" #name "\" ended with an unexpected exception: " << e.what() << std::endl; \
            return false; \
        } catch (...) { \
            std::cout << "Test case \"" #name "\" ended with an unknown exception\n" << std::endl; \
            return false; \
        } \
    } \
    static TestRegistrar name ## _registrar(&name); \
    static bool name ## _inner()

#endif
