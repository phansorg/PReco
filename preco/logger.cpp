#include "logger.h"

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include "spdlog/sinks/daily_file_sink.h"
#include <spdlog/async.h>

void init_logger(const std::string& file_path) {
    spdlog::init_thread_pool(8192, 1);

    const auto stdout_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt >();
    stdout_sink->set_level(spdlog::level::trace);

    // Create a daily logger - a new file is created every day on 0:00am
    const auto daily_sink = std::make_shared<spdlog::sinks::daily_file_sink_mt>(file_path, 0, 0);
    daily_sink->set_level(spdlog::level::trace);

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

    spdlog::register_logger(logger);
}
