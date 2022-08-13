#include "logger.h"

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include "spdlog/sinks/daily_file_sink.h"
#include <spdlog/async.h>

void initLogger(std::string filePath) {
    spdlog::init_thread_pool(8192, 1);

    auto stdout_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt >();
    stdout_sink->set_level(spdlog::level::trace);

    // Create a daily logger - a new file is created every day on 0:00am
    auto daily_sink = std::make_shared<spdlog::sinks::daily_file_sink_mt>(filePath, 0, 0);
    daily_sink->set_level(spdlog::level::trace);

    std::vector<spdlog::sink_ptr> sinks{
        stdout_sink,
        daily_sink
    };

    auto logger = std::make_shared<spdlog::async_logger>(
        LOGGER_MAIN,
        sinks.begin(),
        sinks.end(),
        spdlog::thread_pool(),
        spdlog::async_overflow_policy::block);

    spdlog::register_logger(logger);
}
