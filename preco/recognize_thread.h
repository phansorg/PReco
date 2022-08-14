#pragma once

class recognize_thread
{
	bool thread_loop_;

public:
	recognize_thread();

	void run() const;
	void request_end();
};

