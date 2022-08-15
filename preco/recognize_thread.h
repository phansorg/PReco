#pragma once

#include <opencv2/opencv.hpp>
#include <queue>
#include <mutex>
#include "player.h"

enum class recognize_mode {
	wait_character_select,
	wait_reset,
	wait_init,
};

class recognize_thread
{
	const unsigned long long mat_queue_max_size_ = 10;

	bool thread_loop_;
	int cur_no_;

	bool debug_write_;
	std::string debug_path_;
	recognize_mode mode_;
	int field_width_;
	int field_height_;
	int field_y_;

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

	void wait_character_select(const cv::Mat& org_mat);
	void wait_reset(const cv::Mat& org_mat);
	void debug_init_game(const cv::Mat& org_mat) const;
};

