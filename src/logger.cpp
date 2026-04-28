#include "logger.hpp"
#include "concurrent/thread_pool.hpp"

#include <chrono>

namespace logger {

static std::string_view to_string(Level lvl)
{
    switch (lvl) {
        case Level::Error:   return "[ERROR]";
        case Level::Warning: return "[WARNING]";
        case Level::Info:    return "[INFO]";
        case Level::Debug:   return "[DEBUG]";
        default:             return "[UNKNOWN]";
    }
}

std::string get_current_time() {
    using namespace std::chrono;
    auto now = floor<seconds>(system_clock::now());
    return std::format("{:%Y-%m-%d %H:%M:%S}", now);
}

void detail::write(Level msg_lvl, std::string_view msg)
{
    if (msg_lvl > current_level) {
        return;
    }

    if (current_options.show_timestamp) {
        *current_out << "[" << get_current_time() << "] ";
    }

    if (current_options.show_thread_id) {
        *current_out << "[Thread-" << concurrent::get_short_thread_id() << "] ";
    }

    *current_out << to_string(msg_lvl) << " " << msg << std::endl;
}

void set_options(Options&& options)
{
    current_options = std::move(options);
}

void set_level(Level new_level)
{
    current_level = new_level;
}

void set_log_file(const std::string& filepath)
{
    log_file.open(filepath, std::ios::app);
    if (log_file.is_open()) {
        current_out = &log_file;
    }
}

} // end of `log` namespace
