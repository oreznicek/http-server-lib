#include "logger.hpp"
#include "concurrent/thread_pool.hpp"

#include <chrono>
#include <fstream>
#include <iostream>
#include <sstream>

namespace logger {

static std::ostream* current_out = &std::cout;
static std::mutex out_mutex;
static std::fstream log_file;
static Level current_level = Level::Info;
static Options current_options;

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

static std::string get_current_time() {
    using namespace std::chrono;
    auto now = floor<seconds>(system_clock::now());
    return std::format("{:%Y-%m-%d %H:%M:%S}", now);
}

void detail::write(Level msg_lvl, std::string_view msg)
{
    if (msg_lvl > current_level) {
        return;
    }

    std::ostringstream local_buffer;

    if (current_options.show_timestamp) {
        local_buffer << "[" << get_current_time() << "] ";
    }
    if (current_options.show_thread_id) {
        local_buffer << "[Thread-" << concurrent::get_short_thread_id() << "] ";
    }
    local_buffer << to_string(msg_lvl) << " " << msg << std::endl;

    std::string full_log_line = local_buffer.str();

    {
        std::unique_lock<std::mutex> lock(out_mutex);
        *current_out << full_log_line;
    }
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
