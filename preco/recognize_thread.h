#pragma once

#include <opencv2/opencv.hpp>
#include <queue>
#include <mutex>

class recognize_thread
{
	const unsigned long long mat_queue_max_size_ = 10;

	bool thread_loop_;

	int cur_no_;
	bool debug_mode_;
	std::string debug_path_;
	int field_width_;
	int field_height_;
	int field_x_;
	int field_y_;

	bool capture_end_;

	std::mutex mat_queue_mutex_;
	std::queue<cv::Mat> mat_queue_;

public:
	recognize_thread();

	void run();
	void request_end();
	void set_capture_end();

	[[nodiscard]] bool is_mat_queue_max() const;
	void add_mat_queue(const cv::Mat& mat);

private:
	void process();
	cv::Mat pop();

	void debug_init_game(const cv::Mat& org_mat);
};

