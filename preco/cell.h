#pragma once

#include <opencv2/core/types.hpp>

#include "ring_buffer.h"

class cell
{
public:
	cell();
	void set_rect(cv::Rect in_rect);

	cv::Rect rect;
	cv::Rect recognize_rect;
	ring_buffer mse_ring_buffer;

};
