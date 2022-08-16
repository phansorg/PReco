#pragma once

#include <opencv2/opencv.hpp>
#include <queue>
#include <mutex>
#include "player.h"

enum class game_mode {
	wait_character_selection,
	wait_start,
	wait_init,
};

class recognize_thread
{
	const unsigned long long mat_queue_max_size_ = 10;

	bool thread_loop_;
	int cur_no_;

	bool debug_write_;
	std::string debug_path_;
	game_mode mode_;

	bool capture_end_;

	std::mutex mat_queue_mutex_;
	std::queue<cv::Mat> mat_queue_;

	std::vector<std::unique_ptr<player>> players_;

public:
	recognize_thread();

	static constexpr int b = 0;
	static constexpr int g = 1;
	static constexpr int r = 2;

	void run();
	void request_end();
	void set_capture_end();

	[[nodiscard]] bool is_mat_queue_max() const;
	void add_mat_queue(const cv::Mat& mat);

private:
	void process();
	cv::Mat pop();

	void wait_character_selection(const cv::Mat& org_mat);
	void wait_start(const cv::Mat& org_mat);
	void debug_wait_init(const cv::Mat& org_mat) const;
};

