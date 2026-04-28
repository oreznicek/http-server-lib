#ifndef _LOGGER_HPP
#define _LOGGER_HPP

#include <iostream>
#include <format>
#include <fstream>

namespace logger {

enum class Level {
    Error,
    Warning,
    Info,
    Debug
};

struct Options {
    bool show_timestamp = true;
    bool show_thread_id = true;
    // TODO: bool log_client_ip = true;
    // TODO: bool log_processing_time = false;
};

namespace {
    std::ostream* current_out = &std::cout;
    std::fstream log_file;
    Level current_level = Level::Info;
    Options current_options;

}

namespace detail {
    void write(Level msg_lvl, std::string_view msg);
}

template <typename... Args>
void error(std::format_string<Args...> fmt, Args&&... args) {
    detail::write(Level::Error, std::format(fmt, std::forward<Args>(args)...));
}
template <typename... Args>
void warning(std::format_string<Args...> fmt, Args&&... args) {
    detail::write(Level::Warning, std::format(fmt, std::forward<Args>(args)...));
}
template <typename... Args>
void info(std::format_string<Args...> fmt, Args&&... args) {
    detail::write(Level::Info, std::format(fmt, std::forward<Args>(args)...));
}
template <typename... Args>
void debug(std::format_string<Args...> fmt, Args&&... args) {
    detail::write(Level::Debug, std::format(fmt, std::forward<Args>(args)...));
}

void set_options(Options&& options);
void set_level(Level new_level);
void set_log_file(const std::string& filepath);

} // end of `log` namespace

#endif
