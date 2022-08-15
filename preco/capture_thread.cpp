#include "capture_thread.h"
#include "base_thread.h"

#include "settings.h"
#include "logger.h"

#include <filesystem>

capture_thread::capture_thread(const std::shared_ptr<recognize_thread>& recognize_thread_ptr) {

	thread_loop_ = true;

	recognize_thread_ptr_ = recognize_thread_ptr;

	auto& json = settings::get_instance()->json;
	mode_ = static_cast<capture_mode>(json["capture_mode"].get<int>());
	path_ = json["capture_path"].get<std::string>();
	start_no_ = json["capture_start_no"].get<int>();
	last_no_ = json["capture_last_no"].get<int>();

	cur_no_ = start_no_;
}

void capture_thread::run()
{
	const auto logger = spdlog::get(logger_main);
	SPDLOG_LOGGER_DEBUG(logger, "start");

	while (thread_loop_)
    {
		process();
		SPDLOG_LOGGER_TRACE(logger, "sleep");
		std::this_thread::sleep_for(std::chrono::milliseconds(thread_sleep_ms));
    }

	// メインスレッドのキー押下ではなく、
	// 自スレッドでループ終了条件を満たした場合の通知
	recognize_thread_ptr_->set_capture_end();

	SPDLOG_LOGGER_DEBUG(logger, "end");
}

void capture_thread::request_end()
{
    thread_loop_ = false;
}

void capture_thread::process()
{
	switch(mode_)
	{
	case capture_mode::jpeg:
		read_jpeg();
		break;
	}
}

void capture_thread::read_jpeg()
{
	const auto logger = spdlog::get(logger_main);

	for (; cur_no_ <= last_no_; cur_no_++)
	{
		// 認識スレッドのキューが埋まっている場合、一旦抜ける
		if (recognize_thread_ptr_->is_mat_queue_max())
		{
			return;
		}

		// jpegファイル名
		std::ostringstream zero_padding;
		zero_padding << std::setfill('0') << std::setw(jpeg_file_zero_count_) << cur_no_ << ".jpg";

		// ディレクトリパスとjpegファイル名を結合
		std::filesystem::path file_path = path_;
		file_path.append(zero_padding.str());

		// jpegファイル読み込み
		auto mat = cv::imread(file_path.string());
	}

	// 全ファイル処理した場合、スレッドループ終了
	request_end();
}
