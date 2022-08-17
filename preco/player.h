#pragma once

#include <list>
#include <opencv2/core/types.hpp>

#include "ring_buffer.h"

class player
{
public:
	static constexpr int p1 = 0;
	static constexpr int p2 = 1;
	static constexpr int rows = 12;
	static constexpr int cols = 6;
	static constexpr int nxt_cells = 4;

	static constexpr int b = 0;
	static constexpr int g = 1;
	static constexpr int r = 2;

	static constexpr int mse_init = 10000;

private:
	int player_idx_;

	cv::Rect field_frame_rect_;
	int cell_width_;
	int cell_height_;
	cv::Rect field_cell_rects_[rows][cols];
	cv::Rect field_recognize_rects_[rows][cols];

	cv::Rect nxt_cell_rects_[nxt_cells];
	cv::Rect nxt_recognize_rects_[nxt_cells];

	cv::Rect wait_character_selection_rect_;
	cv::Rect wait_reset_rect_;
	cv::Rect wait_end_rect_;

	int histories_size_;
	ring_buffer nxt_mse_ring_buffer_;

	static cv::Rect to_recognize_rect(cv::Rect cell);
	void init_field_recognize_rects();
	void init_wait_character_selection_rect();
	void init_wait_reset_rect();
	void init_wait_end_rect();

public:
	explicit player(int player_idx);

	[[nodiscard]] bool wait_character_selection(const cv::Mat& org_mat) const;
	[[nodiscard]] bool wait_game_start(const cv::Mat& org_mat) const;
	[[nodiscard]] bool game(int cur_no, const cv::Mat& org_mat, const std::list<cv::Mat>& mat_histories);
	[[nodiscard]] bool wait_nxt_stable(const cv::Mat& org_mat, const std::list<cv::Mat>& mat_histories);
	void debug_render(const cv::Mat& debug_mat) const;
	void render_rect(const cv::Mat& debug_mat, cv::Rect rect, const cv::Scalar& color) const;

};
