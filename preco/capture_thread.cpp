#include "capture_thread.h"
#include "base_thread.h"

#include "settings.h"
#include "logger.h"

#include <filesystem>

CaptureThread::CaptureThread(const std::shared_ptr<game_thread>& game_thread_ptr) {

	thread_loop_ = true;

	game_thread_ptr_ = game_thread_ptr;

	auto& json = Settings::get_instance()->json_;
	mode_ = static_cast<CaptureMode>(json["capture_mode"].get<int>());
	path_ = json["capture_path"].get<std::string>();
	start_no_ = json["capture_start_no"].get<int>();
	last_no_ = json["capture_last_no"].get<int>();

	cur_no_ = start_no_;
}

void CaptureThread::run()
{
	const auto logger = spdlog::get(kLoggerMain);
	SPDLOG_LOGGER_DEBUG(logger, "start");

	while (thread_loop_)
    {
		process();
		SPDLOG_LOGGER_TRACE(logger, "sleep");
		std::this_thread::sleep_for(std::chrono::milliseconds(kThreadSleepMs));
    }

	// メインスレッドのキー押下ではなく、
	// 自スレッドでループ終了条件を満たした場合の通知
	game_thread_ptr_->set_capture_end();

	SPDLOG_LOGGER_DEBUG(logger, "end");
}

void CaptureThread::request_end()
{
    thread_loop_ = false;
}

void CaptureThread::process()
{
	switch(mode_)
	{
	case CaptureMode::kJpeg:
		read_jpeg();
		break;
	}
}

void CaptureThread::read_jpeg()
{
	const auto logger = spdlog::get(kLoggerMain);

	for (; cur_no_ <= last_no_; cur_no_++)
	{
		// 後続スレッドのキューが埋まっている場合、一旦抜ける
		if (game_thread_ptr_->is_capture_mat_queue_max())
		{
			return;
		}

		// jpegファイル名
		std::ostringstream zero_padding;
		//zero_padding << std::setfill('0') << std::setw(jpeg_file_zero_count_) << cur_no_ << ".jpg";
		zero_padding << cur_no_ << ".jpg";

		// ディレクトリパスとjpegファイル名を結合
		std::filesystem::path file_path = path_;
		file_path.append(zero_padding.str());

		// jpegファイル読み込み
		auto mat = cv::imread(file_path.string());

		// 後続スレッドのキューに追加
		game_thread_ptr_->add_capture_mat_queue(mat);
	}

	// 全ファイル処理した場合、スレッドループ終了
	request_end();
}
