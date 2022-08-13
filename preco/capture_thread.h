#pragma once
#include <string>

enum class capture_mode { 
	jpeg = 0,
};

class capture_thread
{
public:
	capture_mode mode;
	std::string path;
	int start_no;
	int last_no;

	bool thread_loop;

	capture_thread(const capture_mode mode, const std::string& path, const int start_no, const int last_no) {
		this->mode = mode;
		this->path = path;
		this->start_no = start_no;
		this->last_no = last_no;

		thread_loop = true;
	}

	void run() const;
	void request_stop();

};

