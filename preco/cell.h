#pragma once

#include <opencv2/core/types.hpp>

#include "ring_buffer.h"

class cell
{
public:
	cell();

	cv::Rect rect;
	cv::Rect recognize_rect;

	void set_rect(cv::Rect in_rect);
	void set_mse(int mse);
	[[nodiscard]] int get_mse() const;

private:
	ring_buffer mse_ring_buffer_;

};
