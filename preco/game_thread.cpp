#include "game_thread.h"
#include "base_thread.h"

#include "settings.h"
#include "logger.h"

game_thread::game_thread()
{
	thread_loop_ = true;

	auto& json = settings::get_instance()->json;
	cur_no_ = json["capture_start_no"].get<int>();
	debug_write_ = json["game_debug_write"].get<bool>();
	debug_path_ = json["game_debug_path"].get<std::string>();
	game_mode_ = static_cast<game_mode>(json["game_start_mode"].get<int>());
	if (game_mode_ == game_mode::wait_character_selection)
	{
		first_game_ = true;
	}
	else
	{
		first_game_ = false;
	}
	game_start_no_ = 0;

	capture_end_ = false;

	players_.push_back(std::make_unique<player>(player::p1));
	players_.push_back(std::make_unique<player>(player::p2));
}

void game_thread::run()
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

void game_thread::request_end()
{
	thread_loop_ = false;
}

void game_thread::set_capture_end()
{
	capture_end_ = true;
}

bool game_thread::is_capture_mat_queue_max() const
{
	return capture_mat_queue_.size() >= capture_mat_queue_max_;
}

void game_thread::add_capture_mat_queue(const cv::Mat& mat)
{
	std::lock_guard lock(capture_mat_queue_mutex_);
	capture_mat_queue_.push(mat);
}

void game_thread::process()
{
	while (!capture_mat_queue_.empty())
	{
		cv::Mat org_mat = pop();

		switch (game_mode_)
		{
		case game_mode::wait_character_selection:
			wait_character_selection(org_mat);
			break;
		case game_mode::wait_game_reset:
			wait_game_reset(org_mat);
			break;
		case game_mode::wait_game_init:
			wait_game_init(org_mat);
			debug_render(org_mat);
			break;
		case game_mode::wait_game_start:
			wait_game_start();
			debug_render(org_mat);
			break;
		case game_mode::wait_game_end:
			wait_game_end(org_mat);
			debug_render(org_mat);
			break;
		}

		add_history(org_mat);
		cur_no_++;
	}

	if (capture_end_ && capture_mat_queue_.empty())
	{
		request_end();
	}
}

cv::Mat game_thread::pop()
{
	std::lock_guard lock(capture_mat_queue_mutex_);
	cv::Mat org_mat = capture_mat_queue_.front();
	capture_mat_queue_.pop();
	return org_mat;
}

void game_thread::add_history(const cv::Mat& org_mat)
{
	mat_histories_.push_front(org_mat);
	if (mat_histories_.size() > settings::history_max)
	{
		mat_histories_.pop_back();
	}
}

// ============================================================
// game
// ============================================================

void game_thread::wait_character_selection(const cv::Mat& org_mat)
{
	for (const auto & player : players_)
	{
		if (!player->wait_character_selection(org_mat)) return;
	}

	const auto logger = spdlog::get(logger_main);
	game_mode_ = game_mode::wait_game_reset;
	logger->info("No:{} game_mode:{}", cur_no_, static_cast<int>(game_mode_));
}

void game_thread::wait_game_reset(const cv::Mat& org_mat)
{
	for (const auto& player : players_)
	{
		if (!player->wait_game_reset(org_mat)) return;
	}

	const auto logger = spdlog::get(logger_main);
	game_mode_ = game_mode::wait_game_init;
	logger->info("No:{} game_mode:{}", cur_no_, static_cast<int>(game_mode_));
}

void game_thread::wait_game_init(const cv::Mat& org_mat)
{
	for (const auto& player : players_)
	{
		if (!player->wait_game_init(org_mat, mat_histories_)) return;
	}

	if (first_game_)
	{
		first_game_ = false;
		game_start_no_ = cur_no_ + wait_first_game_start_span_;
	}
	else
	{
		game_start_no_ = cur_no_ + wait_game_start_span_;
	}
	game_mode_ = game_mode::wait_game_start;
	const auto logger = spdlog::get(logger_main);
	logger->info("No:{} game_mode:{}", cur_no_, static_cast<int>(game_mode_));
}

void game_thread::wait_game_start()
{
	if (game_start_no_ != cur_no_) return;

	const auto logger = spdlog::get(logger_main);
	game_mode_ = game_mode::wait_game_end;
	logger->info("No:{} game_mode:{}", cur_no_, static_cast<int>(game_mode_));
}

void game_thread::wait_game_end(const cv::Mat& org_mat)
{
	const auto logger = spdlog::get(logger_main);
	SPDLOG_LOGGER_TRACE(logger, "game No:{}", cur_no_);

	bool game_end = false;
	for (const auto& player : players_)
	{
		game_end |= player->game(cur_no_, org_mat, mat_histories_);
	}
	if (!game_end) return;

	for (const auto& player : players_)
	{
		player->game_end();
	}

	game_mode_ = game_mode::wait_game_reset;
	logger->info("No:{} game_mode:{}", cur_no_, static_cast<int>(game_mode_));
}


// ============================================================
// debug
// ============================================================

void game_thread::debug_render(const cv::Mat& org_mat) const
{
	if (!settings::debug) return;

	const auto debug_mat = org_mat.clone();
	for (const auto& player : players_)
	{
		player->debug_render(debug_mat);
	}

	// 出力ファイル名
	std::ostringstream file_name;
	file_name << cur_no_ << ".png";

	// ディレクトリパスと出力ファイル名を結合
	std::filesystem::path file_path = debug_path_;
	file_path.append(file_name.str());

	// ファイル出力
	imwrite(file_path.string(), debug_mat);
}
