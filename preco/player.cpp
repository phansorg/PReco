#include "player.h"

#include <opencv2/imgproc.hpp>

#include "settings.h"
#include "logger.h"

player::player(const int player_idx)
{
	player_idx_ = player_idx;
	player_mode_ = player_mode::wait_game_start;
	cur_next_idx_ = 0;

	// field
	auto& json = settings::get_instance()->json;
	field_frame_rect_ = cv::Rect(
		json["player_field_x"][player_idx_].get<int>(),
		json["player_field_y"].get<int>(),
		json["player_field_w"].get<int>(),
		json["player_field_h"].get<int>()
	);
	cell_width_ = field_frame_rect_.width / cols;
	cell_height_ = field_frame_rect_.height / rows;

	// nxt1
	nxt_cells_[0][axis].set_rect(cv::Rect(
		json["player_nxt1_x"][player_idx_].get<int>(),
		json["player_nxt1_y"].get<int>(),
		cell_width_,
		cell_height_
	));
	auto clone_rect = nxt_cells_[0][axis].frame_rect;
	clone_rect.y += clone_rect.height;
	nxt_cells_[0][child].set_rect(clone_rect);

	// nxt2
	nxt_cells_[1][axis].set_rect(cv::Rect(
		json["player_nxt2_x"][player_idx_].get<int>(),
		json["player_nxt2_y"].get<int>(),
		cell_width_ * 4 / 5,
		cell_height_ * 4 / 5
	));
	clone_rect = nxt_cells_[1][axis].frame_rect;
	clone_rect.y += clone_rect.height;
	nxt_cells_[1][child].set_rect(clone_rect);

	// combo
	combo_cell_.set_rect(cv::Rect(
		json["player_score_x"][player_idx_].get<int>() + 4,
		json["player_score_y"].get<int>() + 1,
		cell_width_ / 2,
		cell_height_ / 4
	));

	// その他Rectの計算
	init_field_cells();
	init_combo_cell();
	init_end_cell();
	init_wait_character_selection_rect();
	init_wait_reset_rect();
}

// ============================================================
// rect
// ============================================================
void player::init_field_cells()
{
	const auto width = field_frame_rect_.width;
	const auto height = field_frame_rect_.height;
	for (int row = 0; row < rows; row++)
	{
		const auto y1 = height * row / rows + field_frame_rect_.y;
		const auto y2 = height * (row + 1) / rows + field_frame_rect_.y;
		for (int col = 0; col < cols; col++)
		{
			const auto x1 = width * col / cols + field_frame_rect_.x;
			const auto x2 = width * (col + 1) / cols + field_frame_rect_.x;
			field_cells_[row][col].set_rect(cv::Rect(
				x1,
				y1,
				x2 - x1,
				y2 - y1
			));
		}
	}
}

void player::init_combo_cell()
{
	// スコアの左端が変化したらcombo
	const auto width = cell_width_ / 2;
	const auto x = player_idx_ == p1 ?
		field_frame_rect_.x : field_frame_rect_.x + field_frame_rect_.width - width;
	end_cell_.set_rect(cv::Rect(
		x,
		field_frame_rect_.y + field_frame_rect_.height + width / 2,
		width,
		width
	));
}

void player::init_end_cell()
{
	// 角の領域が全て緑であればend
	const auto width = cell_width_ / 2;
	const auto x = player_idx_ == p1 ?
		field_frame_rect_.x : field_frame_rect_.x + field_frame_rect_.width - width;
	end_cell_.set_rect(cv::Rect(
		x,
		field_frame_rect_.y + field_frame_rect_.height + width / 2,
		width,
		width
	));
}

void player::init_wait_character_selection_rect()
{
	// 1P盤面の上半分領域が全て赤であればOK
	// 2P盤面の下半分領域が全て緑であればOK
	const auto y = player_idx_ == p1 ? 
		field_frame_rect_.y : field_frame_rect_.y + field_frame_rect_.height / 2;
	wait_character_selection_rect_ = cv::Rect(
		field_frame_rect_.x,
		y,
		field_frame_rect_.width,
		field_frame_rect_.height / 2
	);
}

void player::init_wait_reset_rect()
{
	// 盤面の中央領域が全て黒であればOK
	wait_reset_rect_ = cv::Rect(
		field_frame_rect_.x + field_frame_rect_.width / 2,
		field_frame_rect_.y + field_frame_rect_.height / 2,
		cell_width_,
		cell_height_
	);
}

// ============================================================
// game
// ============================================================

bool player::wait_character_selection(const cv::Mat& org_mat) const
{
	std::vector<cv::Mat> channels;
	cv::Point* pos = nullptr;

	const auto roi_mat = org_mat(wait_reset_rect_);
	split(roi_mat, channels);
	if (player_idx_ == p1)
	{
		// 1P盤面の上半分領域が全て赤であればOK
		if (!checkRange(channels[cell::b], true, pos, 0, 100)) return false;
		if (!checkRange(channels[cell::g], true, pos, 0, 100)) return false;
		if (!checkRange(channels[cell::r], true, pos, 180, 255)) return false;
	}
	else
	{
		// 2P盤面の下半分領域が全て緑であればOK
		if (!checkRange(channels[cell::b], true, pos, 0, 100)) return false;
		if (!checkRange(channels[cell::g], true, pos, 180, 255)) return false;
		if (!checkRange(channels[cell::r], true, pos, 0, 100)) return false;
	}

	return true;
}

bool player::wait_game_reset(const cv::Mat& org_mat)
{
	// 盤面の中央領域が全て黒であればOK
	cv::Point* pos = nullptr;
	if (const auto roi_mat = org_mat(wait_reset_rect_);
		!checkRange(roi_mat, true, pos, 0, 30)) return false;

	// fieldをリセット
	for (auto& field_row : field_cells_)
	{
		for (auto& field_cell : field_row)
		{
			field_cell.reset();
		}
	}

	// nxtをリセット
	for (auto& nxt_child_cells : nxt_cells_)
	{
		for (auto& nxt_cell : nxt_child_cells)
		{
			nxt_cell.reset();
		}
	}
	nxt_records_.clear();

	// comboをリセット
	combo_cell_.reset();

	// endをリセット
	end_cell_.reset();

	return true;
}

bool player::wait_game_init(const cv::Mat& org_mat, const std::list<cv::Mat>& mat_histories)
{

	// 初期化済みの場合、OK
	if (!nxt_records_.empty())
	{
		const auto logger = spdlog::get(logger_main);
		SPDLOG_LOGGER_TRACE(logger, "wait_game_init ok p:{}", player_idx_);
		return true;
	}

	// nxtが全て安定し、色があればOK
	update_nxt_cells(org_mat, mat_histories);
	for (auto& nxt_child_cells : nxt_cells_)
	{
		for (auto& nxt_cell : nxt_child_cells)
		{
			if (!nxt_cell.is_stabilized()) return false;
			if (nxt_cell.get_recognize_color() == color::none) return false;
		}
	}

	// 初期化完了時のnxtを登録し、再度リセット(game_startで再度認識させるため)
	for (auto& nxt_child_cells : nxt_cells_)
	{
		for (auto& nxt_cell : nxt_child_cells)
		{
			nxt_records_.push_back(nxt_cell.get_recognize_color());
			nxt_cell.reset();
		}
	}
	cur_next_idx_ = 0;

	// player_modeをゲーム開始後にセット
	const auto logger = spdlog::get(logger_main);
	player_mode_ = player_mode::wait_nxt_stabilize;
	logger->info("p:{} player_mode:{}", player_idx_, static_cast<int>(player_mode_));

	return true;
}

bool player::game(int cur_no, const cv::Mat& org_mat, const std::list<cv::Mat>& mat_histories)
{
	update_all_cells(org_mat, mat_histories);

	switch (player_mode_)
	{
	case player_mode::wait_game_start:
		break;
	case player_mode::wait_nxt_stabilize:
		wait_nxt_stabilize();
		break;
	case player_mode::wait_nxt_change:
		wait_nxt_change();
		break;
	}

	return wait_game_end();
}

void player::wait_nxt_stabilize()
{
	for (auto& nxt_child_cells : nxt_cells_)
	{
		for (auto& nxt_cell : nxt_child_cells)
		{
			if (!nxt_cell.is_stabilized()) return;
		}
	}

	const auto logger = spdlog::get(logger_main);
	player_mode_ = player_mode::wait_nxt_change;
	logger->info("p:{} player_mode:{}", player_idx_, static_cast<int>(player_mode_));
}

void player::wait_nxt_change()
{
	for (auto& nxt_child_cells : nxt_cells_)
	{
		for (auto& nxt_cell : nxt_child_cells)
		{
			if (nxt_cell.is_stabilized()) return;
		}
	}

	const auto logger = spdlog::get(logger_main);
	player_mode_ = player_mode::wait_nxt_stabilize;
	logger->info("p:{} player_mode:{}", player_idx_, static_cast<int>(player_mode_));
}

bool player::wait_game_end() const
{
	return end_cell_.get_recognize_color() == color::g;
}

void player::update_all_cells(const cv::Mat& org_mat, const std::list<cv::Mat>& mat_histories)
{
	// field
	for (auto& field_row : field_cells_)
	{
		for (auto& field_cell : field_row)
		{
			update_cell(org_mat, mat_histories, field_cell);
		}
	}

	// nxt
	update_nxt_cells(org_mat, mat_histories);

	// combo
	update_cell(org_mat, mat_histories, combo_cell_);

	// end
	update_cell(org_mat, mat_histories, end_cell_);
}

void player::update_nxt_cells(const cv::Mat& org_mat, const std::list<cv::Mat>& mat_histories)
{
	for (auto& nxt_child_cells : nxt_cells_)
	{
		for (auto& nxt_cell : nxt_child_cells)
		{
			update_cell(org_mat, mat_histories, nxt_cell);
		}
	}
}

void player::update_cell(const cv::Mat& org_mat, const std::list<cv::Mat>& mat_histories, cell& target_cell) const
{
	// 対象領域を取得し、HSVに変換
	const auto& org_roi = org_mat(target_cell.recognize_rect);
	cv::Mat org_hsv_mat;
	cvtColor(org_roi, org_hsv_mat, cv::COLOR_BGR2HSV);

	const auto history_roi = mat_histories.front()(target_cell.recognize_rect);
	cv::Mat history_hsv_mat;
	cvtColor(history_roi, history_hsv_mat, cv::COLOR_BGR2HSV);

	// 対象領域のピクセル毎にMSEを計算
	cv::Mat diff_mat;
	subtract(org_hsv_mat, history_hsv_mat, diff_mat);
	cv::Mat pow_mat;
	pow(diff_mat, 2, pow_mat);

	// MSEの平均をとり、cellに設定
	const auto before_stabilized = target_cell.is_stabilized();
	target_cell.set_mse(mean(pow_mat));

	// 安定状態に遷移した場合、色を更新
	if (!before_stabilized && target_cell.is_stabilized())
	{
		target_cell.update_recognize_color(mean(org_roi));
	}
}

void player::game_end()
{
	const auto logger = spdlog::get(logger_main);
	player_mode_ = player_mode::wait_game_start;
	logger->info("p:{} player_mode:{}", player_idx_, static_cast<int>(player_mode_));
}

// ============================================================
// debug
// ============================================================

void player::debug_render(const cv::Mat& debug_mat) const
{
	const auto logger = spdlog::get(logger_main);

	// fieldの線を描画
	for (const auto& field_row : field_cells_)
	{
		for (const auto& field_cell : field_row)
		{
			field_cell.debug_render(debug_mat);
		}
	}
	// nxtの線を描画
	for (const auto& nxt_child_cells : nxt_cells_)
	{
		for (const auto& nxt_cell : nxt_child_cells)
		{
			nxt_cell.debug_render(debug_mat);
		}
	}

	// comboの線を描画
	combo_cell_.debug_render(debug_mat);

	// endの線を描画
	end_cell_.debug_render(debug_mat);

	// nxtのmse
	SPDLOG_LOGGER_TRACE(logger, "game p:{} mse1:{} mse2:{} mse3:{} mse4:{}",
		player_idx_,
		nxt_cells_[0][axis].get_mse(),
		nxt_cells_[0][child].get_mse(),
		nxt_cells_[1][axis].get_mse(),
		nxt_cells_[1][child].get_mse()
	);
}


