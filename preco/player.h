#pragma once

#include <opencv2/core/types.hpp>

class player
{
	int player_idx_;

	cv::Rect field_frame_rect_;
	cv::Rect draw1_frame_rect_;
	cv::Rect draw2_frame_rect_;

	std::vector<cv::Rect> draw_cell_rects_;
	cv::Rect wait_character_selection_rect_;
	cv::Rect wait_reset_rect_;

	void init_draw_cell_rects();
	void init_wait_character_selection_rect();
	void init_wait_reset_rect();

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
