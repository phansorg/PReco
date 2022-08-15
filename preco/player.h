#pragma once

#include <opencv2/core/types.hpp>

class player
{
	int player_idx_;

	int field_x_;
	int field_y_;
	int field_width_;
	int field_height_;

	void init_field_rect();
	void init_wait_character_rect();
	void init_wait_reset_rect();

public:
	explicit player(int player_idx);

	static constexpr int p1 = 0;
	static constexpr int p2 = 1;

	cv::Rect field_rect;
	std::vector<cv::Rect> next_rect_vector;
	cv::Rect wait_character_rect;
	cv::Rect wait_reset_rect;
};
