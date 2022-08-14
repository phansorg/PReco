#include "capture_thread.h"
#include "logger.h"

capture_thread::capture_thread(
	const std::shared_ptr<recognize_thread>& recognize_thread_ptr,
	const capture_mode mode,
	const std::string& path,
	const int start_no,
	const int last_no) {

	thread_loop_ = true;

	recognize_thread_ptr_ = recognize_thread_ptr;
	mode_ = mode;
	path_ = path;
	start_no_ = start_no;
	last_no_ = last_no;
}


void capture_thread::run() const
{
	const auto logger = spdlog::get(logger_main);
	SPDLOG_LOGGER_DEBUG(logger, "start");

	while (thread_loop_)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

	// メインスレッドのキー押下ではなく、
	// 自スレッドでループ終了条件を満たした場合の認識スレッド停止要求
	recognize_thread_ptr_->request_end();

	SPDLOG_LOGGER_DEBUG(logger, "end");
}

void capture_thread::request_end()
{
    thread_loop_ = false;
}
