#pragma once
#include <memory>
#include <string>

#include <opencv2/opencv.hpp>

#include "recognize_thread.h"

enum class capture_mode { 
	jpeg = 0,
};

class capture_thread
{
	const int jpeg_file_zero_count_ = 7;

	bool thread_loop_;

	std::shared_ptr<recognize_thread> recognize_thread_ptr_;
	capture_mode mode_;
	std::string path_;
	int start_no_;
	int last_no_;
	int cur_no_;

public:
	capture_thread(
		const std::shared_ptr<recognize_thread>& recognize_thread_ptr,
		const capture_mode mode,
		const std::string& path,
		const int start_no,
		const int last_no);

	void run();
	void request_end();

private:
	void process();
	void read_jpeg();
};

