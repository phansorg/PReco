#include "recognize_thread.h"
#include "base_thread.h"
#include "logger.h"

recognize_thread::recognize_thread()
{
	thread_loop_ = true;
	capture_end_ = false;
}

void recognize_thread::run()
{
	const auto logger = spdlog::get(logger_main);
	SPDLOG_LOGGER_DEBUG(logger, "start");

	while (thread_loop_)
	{
		process();
		SPDLOG_LOGGER_TRACE(logger, "sleep");
		std::this_thread::sleep_for(std::chrono::milliseconds(thread_sleep_ms));
	}

	SPDLOG_LOGGER_DEBUG(logger, "end");
}

void recognize_thread::request_end()
{
	thread_loop_ = false;
}

void recognize_thread::set_capture_end()
{
	capture_end_ = true;
}

bool recognize_thread::is_mat_queue_max() const
{
	return mat_queue_.size() >= mat_queue_max_size_;
}

void recognize_thread::add_mat_queue(const cv::Mat& mat)
{
	std::lock_guard lock(mat_queue_mutex_);
	mat_queue_.push(mat);
}

void recognize_thread::process()
{
	while(!mat_queue_.empty())
	{
		cv::Mat mat = mat_queue_.front();
		mat_queue_mutex_.lock();
		mat_queue_.pop();
		mat_queue_mutex_.unlock();
	}

	if (capture_end_&& mat_queue_.empty())
	{
		request_end();
	}
}
