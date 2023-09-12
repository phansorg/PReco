#pragma once

#include <opencv2/opencv.hpp>
#include <queue>
#include <mutex>
#include "player.h"

enum class GameMode {
	kWaitCharacterSelection,
	kWaitGameReset,
	kWaitGameInit,
	kWaitGameStart,
	kWaitGameEnd,
};

class game_thread
{
	const unsigned long long capture_mat_queue_max_ = 10;

	const int wait_game_start_span_ = 67;
	const int wait_first_game_start_span_ = 94;

	bool thread_loop_;
	int cur_no_;

	bool debug_write_;
	std::string debug_path_;
	GameMode game_mode_;
	bool first_game_;
	int game_start_no_;

	bool capture_end_;

	std::mutex capture_mat_queue_mutex_;
	std::queue<cv::Mat> capture_mat_queue_;

	std::vector<std::unique_ptr<Player>> players_;

	std::list<cv::Mat> mat_histories_;
public:
	game_thread();

	void run();
	void request_end();
	void set_capture_end();

	[[nodiscard]] bool is_capture_mat_queue_max() const;
	void add_capture_mat_queue(const cv::Mat& mat);

private:
	void process();
	cv::Mat pop();
	void add_history(const cv::Mat& org_mat);

	void wait_character_selection(const cv::Mat& org_mat);
	void wait_game_reset(const cv::Mat& org_mat);
	void wait_game_init(const cv::Mat& org_mat);
	void wait_game_start();
	void wait_game_end(const cv::Mat& org_mat);

	void debug_render(const cv::Mat& org_mat) const;
};

