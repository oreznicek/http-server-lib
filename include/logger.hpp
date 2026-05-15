#ifndef _LOGGER_HPP
#define _LOGGER_HPP

#include <format>

namespace logger {

/**
 * @brief Defines the severity levels for the logger.
 *
 * @details The logging level determines which messages are processed.
 *          Any log message with a severity higher than the currently
 *          configured global level will be silently discarded.
 */
enum class Level {
    Error,   ///< Critical issues that cause severe failures or prevent execution.
    Warning, ///< Unexpected or dangerous situations that the application can recover from.
    Info,    ///< Standard runtime events and major application milestones.
    Debug    ///< Highly detailed internal state information used by developers for tracing.
};

/**
 * @brief Configuration flags for formatting log output.
 *
 * @details These options control what contextual metadata is automatically
 *          prepended to the log message string before it is written to the sink.
 */
struct Options {
    bool show_timestamp = true; ///< Includes the current system time in the log entry.
    bool show_thread_id = true; ///< Includes the ID of the thread that generated the log.

    // @todo Implement client IP logging for better connection tracing.
    // bool log_client_ip = true;

    // @todo Implement processing time logging to measure route handler latency.
    // bool log_processing_time = false;
};

namespace detail {
    void write(Level msg_lvl, std::string_view msg);
}

/**
 * @brief Logs a critical error message.
 *
 * @tparam Args Types of the arguments to be formatted.
 * @param fmt Format string.
 * @param args The arguments to safely format and insert into the string.
 */
template <typename... Args>
void error(std::format_string<Args...> fmt, Args&&... args) {
    detail::write(Level::Error, std::format(fmt, std::forward<Args>(args)...));
}

/**
 * @brief Logs a warning message.
 *
 * @tparam Args Types of the arguments to be formatted.
 * @param fmt Format string.
 * @param args The arguments to safely format and insert into the string.
 */
template <typename... Args>
void warning(std::format_string<Args...> fmt, Args&&... args) {
    detail::write(Level::Warning, std::format(fmt, std::forward<Args>(args)...));
}

/**
 * @brief Logs an informational message.
 *
 * @tparam Args Types of the arguments to be formatted.
 * @param fmt Format string.
 * @param args The arguments to safely format and insert into the string.
 */
template <typename... Args>
void info(std::format_string<Args...> fmt, Args&&... args) {
    detail::write(Level::Info, std::format(fmt, std::forward<Args>(args)...));
}

/**
 * @brief Logs a diagnostic debug message.
 *
 * @tparam Args Types of the arguments to be formatted.
 * @param fmt Format string.
 * @param args The arguments to safely format and insert into the string.
 */
template <typename... Args>
void debug(std::format_string<Args...> fmt, Args&&... args) {
    detail::write(Level::Debug, std::format(fmt, std::forward<Args>(args)...));
}

void set_options(Options&& options);
void set_level(Level new_level);
void set_log_file(const std::string& filepath);

} // end of `log` namespace

#endif
