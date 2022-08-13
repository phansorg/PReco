#include "logger.h"

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include "spdlog/sinks/daily_file_sink.h"
#include <spdlog/async.h>

constexpr auto q_size = 8192;
constexpr auto thread_count = 1;

constexpr auto rotation_hour = 0;
constexpr auto rotation_minute = 0;
constexpr auto truncate = false;
constexpr auto max_files = 7;

void init_logger(const std::string& file_path) {
    spdlog::init_thread_pool(q_size, thread_count);

    // Init stdout log
    const auto stdout_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt >();
    stdout_sink->set_level(spdlog::level::trace);

    // Init daily file log
    const auto daily_sink = std::make_shared<spdlog::sinks::daily_file_sink_mt>(
        file_path,
        rotation_hour,
        rotation_minute,
        truncate,
        max_files);
    daily_sink->set_level(spdlog::level::trace);

    // Register logger
    std::vector<spdlog::sink_ptr> sinks{
        stdout_sink,
        daily_sink
    };
    const auto logger = std::make_shared<spdlog::async_logger>(
        logger_main,
        sinks.begin(),
        sinks.end(),
        spdlog::thread_pool(),
        spdlog::async_overflow_policy::block);
    logger->set_level(spdlog::level::trace);
    spdlog::register_logger(logger);
}
