#pragma once
#include <memory>
#include <string>
#include <opencv2/opencv.hpp>

#include "game_thread.h"

enum class capture_mode { 
	jpeg = 0,
};

class capture_thread
{
	const int jpeg_file_zero_count_ = 7;

	bool thread_loop_;

	std::shared_ptr<game_thread> game_thread_ptr_;
	capture_mode mode_;
	std::string path_;
	int start_no_;
	int last_no_;

	int cur_no_;

public:
	capture_thread(const std::shared_ptr<game_thread>& game_thread_ptr);

	void run();
	void request_end();

private:
	void process();
	void read_jpeg();
};

