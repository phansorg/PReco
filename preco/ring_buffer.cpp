#include "ring_buffer.h"

ring_buffer::ring_buffer(const int init_value, const int records, const int rows, const int cols)
{
	records_ = records;
	rows_ = rows;
	cols_ = cols;

	cur_record_ = 0;
	record_length_ = rows_ * cols_;
	const auto buffer_size = records_ * rows_ * cols_;
	histories_ = std::make_unique<int[]>(buffer_size);
	for (auto idx = 0; idx < buffer_size; idx++)
	{
		histories_[idx] = init_value;
	}
}

void ring_buffer::reset(const int init_value)
{
	const auto buffer_size = records_ * rows_ * cols_;
	for (auto idx = 0; idx < buffer_size; idx++)
	{
		histories_[idx] = init_value;
	}
}

void ring_buffer::next_record()
{
	cur_record_ = (cur_record_ + 1) % records_;
}

void ring_buffer::prev_record()
{
	cur_record_ = (cur_record_ + records_ - 1) % records_;
}

void ring_buffer::set(const int value, const int row, const int col)
{
	const auto idx = cur_record_ * record_length_ + row * cols_ + col;
	histories_[idx] = value;
}

int ring_buffer::get(const int row, const int col) const
{
	const auto idx = cur_record_ * record_length_ + row * cols_ + col;
	return histories_[idx];
}
