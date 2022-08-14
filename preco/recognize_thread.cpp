#include "recognize_thread.h"
#include "base_thread.h"
#include "logger.h"

recognize_thread::recognize_thread()
{
	thread_loop_ = true;
}

void recognize_thread::run() const
{
	const auto logger = spdlog::get(logger_main);
	SPDLOG_LOGGER_DEBUG(logger, "start");

	while (thread_loop_)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(thread_sleep_ms));
	}

	SPDLOG_LOGGER_DEBUG(logger, "end");
}

void recognize_thread::request_end()
{
	thread_loop_ = false;
}
