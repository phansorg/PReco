#pragma once

#include <filesystem>
#include <list>
#include <opencv2/core/types.hpp>

#include "cell.h"
#include "color_map.h"

enum class PlayerMode {
	kWaitGameStart,
	kWaitNxtStabilize,
	kWaitNxtChange,
	kWaitGameEnd,
};

class Player
{
public:
	static constexpr int p1_ = 0;
	static constexpr int p2_ = 1;
	static constexpr int axis_ = 0;
	static constexpr int child_ = 1;

	static constexpr int rows0_ = 12;
	static constexpr int rows2_ = 14;
	static constexpr int cols_ = 6;
	static constexpr int nxt_max_ = 2;
	static constexpr int nxt_child_max_ = 2;

	struct Operation
	{
		color colors[nxt_child_max_];
		int col;
		int rotate;
	};

private:
	int player_idx_;
	PlayerMode player_mode_;
	int cur_records_idx_;

	std::string history_dir_;
	std::filesystem::path history_path_;
	std::filesystem::path game_record_path_;

	cv::Rect field_frame_rect_;
	int cell_width_;
	int cell_height_;
	Cell field_cells_[rows2_][cols_];

	Cell nxt_cells_[nxt_max_][nxt_child_max_];
	std::vector<Operation> operation_records_;

	Cell end_cell_;
	Cell combo_cell_;

	cv::Rect wait_character_selection_rect_;
	cv::Rect wait_reset_rect_;

	// ééçáêî
	int game_no_;

	ColorMap color_map_;

	int put_nxt_wait_frames_;

public:
	explicit Player(int player_idx);

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
	void game_end();

private:
	void wait_nxt_stabilize();
	bool wait_combo();
	void wait_nxt_change();
	void put_nxt();
	[[nodiscard]] bool wait_game_end() const;

	void update_all_cells(const cv::Mat& org_mat, const std::list<cv::Mat>& mat_histories);
	void update_nxt_cells(const cv::Mat& org_mat, const std::list<cv::Mat>& mat_histories);
	void update_cell(const cv::Mat& org_mat, const std::list<cv::Mat>& mat_histories, Cell& target_cell) const;

	void write_history() const;
	static unsigned char to_color_text(color from_color);
	void to_field_text(unsigned char* buffer) const;
	void append_game_record() const;

public:
	void debug_render(const cv::Mat& debug_mat) const;
};
