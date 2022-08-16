#pragma once

#include <opencv2/core/types.hpp>

class player
{
	int player_idx_;

	int field_x_;
	int field_y_;
	int field_w_;
	int field_h_;
	int draw1_x_;
	int draw1_y_;
	int draw1_w_;
	int draw1_h_;
	int draw2_x_;
	int draw2_y_;
	int draw2_w_;
	int draw2_h_;
	int draw_histories_size_;

	cv::Rect field_rect_;
	std::vector<cv::Rect> draw_rect_vector_;
	cv::Rect wait_character_selection_rect_;
	cv::Rect wait_reset_rect_;

	void init_field_rect();
	void init_draw_rect_vector();
	void init_wait_character_selection_rect();
	void init_wait_reset_rect();

	std::vector<cv::Mat> prev_draw_vector_;

public:
	explicit player(int player_idx);

	static constexpr int p1 = 0;
	static constexpr int p2 = 1;
	static constexpr int rows = 12;
	static constexpr int cols = 6;

	static constexpr int b = 0;
	static constexpr int g = 1;
	static constexpr int r = 2;

	[[nodiscard]] bool wait_character_selection(const cv::Mat& org_mat) const;
	[[nodiscard]] bool wait_game_start(const cv::Mat& org_mat) const;
	void debug_wait_init(const cv::Mat& debug_mat) const;
};
