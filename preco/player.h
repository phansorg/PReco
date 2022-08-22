#pragma once

#include <filesystem>
#include <list>
#include <opencv2/core/types.hpp>

#include "cell.h"

enum class player_mode {
	wait_game_start,
	wait_nxt_stabilize,
	wait_nxt_change,
};

class player
{
public:
	static constexpr int p1 = 0;
	static constexpr int p2 = 1;
	static constexpr int axis = 0;
	static constexpr int child = 1;

	static constexpr int rows0 = 12;
	static constexpr int rows2 = 14;
	static constexpr int cols = 6;
	static constexpr int nxt_max = 2;
	static constexpr int nxt_child_max = 2;

private:
	int player_idx_;
	player_mode player_mode_;
	int cur_nxt_idx_;

	std::string history_dir_;
	std::filesystem::path history_path_;

	cv::Rect field_frame_rect_;
	int cell_width_;
	int cell_height_;
	cell field_cells_[rows2][cols];

	cell nxt_cells_[nxt_max][nxt_child_max];
	std::vector<color> nxt_records_;

	cell end_cell_;
	cell combo_cell_;

	cv::Rect wait_character_selection_rect_;
	cv::Rect wait_reset_rect_;

public:
	explicit player(int player_idx);

private:
	void init_field_cells();
	void init_combo_cell();
	void init_end_cell();

	void init_wait_character_selection_rect();
	void init_wait_reset_rect();

public:
	[[nodiscard]] bool wait_character_selection(const cv::Mat& org_mat) const;
	[[nodiscard]] bool wait_game_reset(const cv::Mat& org_mat);
	[[nodiscard]] bool wait_game_init(const cv::Mat& org_mat, const std::list<cv::Mat>& mat_histories);
	[[nodiscard]] bool game(int cur_no, const cv::Mat& org_mat, const std::list<cv::Mat>& mat_histories);
	[[nodiscard]] bool wait_game_end() const;
	void game_end();

private:
	void wait_nxt_stabilize();
	void wait_nxt_change();

	void update_all_cells(const cv::Mat& org_mat, const std::list<cv::Mat>& mat_histories);
	void update_nxt_cells(const cv::Mat& org_mat, const std::list<cv::Mat>& mat_histories);
	void update_cell(const cv::Mat& org_mat, const std::list<cv::Mat>& mat_histories, cell& target_cell) const;

	void write_history() const;
	static unsigned char to_color_text(color from_color);
	void to_field_text(unsigned char* buffer) const;

public:
	void debug_render(const cv::Mat& debug_mat) const;
};
