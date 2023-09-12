#pragma once
#include <memory>

class RingBuffer
{
	int records_;
	int rows_;
	int cols_;

	int cur_record_;
	int record_length_;
	std::unique_ptr<int[]> histories_;


public:
	explicit RingBuffer(int init_value = 0, int records = 1, int rows = 1, int cols = 1);
	void reset(int init_value = 0);
	void next_record();
	void prev_record();
	void set(int value = 0, int row = 0, int col = 0);
	[[nodiscard]] int get(int row = 0, int col = 0) const;
};

