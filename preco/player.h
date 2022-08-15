#pragma once

#include <opencv2/core/types.hpp>

class player
{
	int player_idx_;

	int field_x_;
	int field_y_;
	int field_w_;
	int field_h_;
	int next1_x_;
	int next1_y_;
	int next1_w_;
	int next1_h_;
	int next2_x_;
	int next2_y_;
	int next2_w_;
	int next2_h_;

	void init_field_rect();
	void init_next_rect_vector();
	void init_wait_character_rect();
	void init_wait_reset_rect();

public:
	explicit player(int player_idx);

	static constexpr int p1 = 0;
	static constexpr int p2 = 1;
	static constexpr int rows = 12;
	static constexpr int cols = 6;

	cv::Rect field_rect;
	std::vector<cv::Rect> next_rect_vector;
	cv::Rect wait_character_rect;
	cv::Rect wait_reset_rect;
};
