#include "test_common.hpp"
#include "net/common.hpp"
#include "logger.hpp"

#include <format>

int main(int argc, char* argv[]) {
    net::Context context;
    logger::set_level(logger::Level::Debug);
    logger::set_log_file(std::format("{}.log", argv[0]));
    return TestRegistry::instance().run_all();
}
