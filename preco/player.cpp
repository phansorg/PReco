#include "player.h"

#include <opencv2/imgproc.hpp>

#include "settings.h"
#include "logger.h"

player::player(const int player_idx)
{
	player_idx_ = player_idx;

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

	// その他Rectの計算
	init_field_cells();
	init_wait_character_selection_rect();
	init_wait_reset_rect();
	init_wait_end_rect();
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

void player::init_wait_end_rect()
{
	// 角の領域が全て緑であればOK
	constexpr auto width = 10;
	const auto x = player_idx_ == p1 ?
		field_frame_rect_.x : field_frame_rect_.x + field_frame_rect_.width - width;
	end_cell_.set_rect(cv::Rect(
		x,
		field_frame_rect_.y + field_frame_rect_.height + width,
		width,
		width
	));
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
		if (!checkRange(channels[b], true, pos, 0, 100)) return false;
		if (!checkRange(channels[g], true, pos, 0, 100)) return false;
		if (!checkRange(channels[r], true, pos, 180, 255)) return false;
	}
	else
	{
		// 2P盤面の下半分領域が全て緑であればOK
		if (!checkRange(channels[b], true, pos, 0, 100)) return false;
		if (!checkRange(channels[g], true, pos, 180, 255)) return false;
		if (!checkRange(channels[r], true, pos, 0, 100)) return false;
	}

	return true;
}

bool player::wait_game_start(const cv::Mat& org_mat) const
{
	// 盤面の中央領域が全て黒であればOK
	cv::Point* pos = nullptr;
	if (const auto roi_mat = org_mat(wait_reset_rect_);
		!checkRange(roi_mat, true, pos, 0, 30)) return false;

	return true;
}

bool player::game(int cur_no, const cv::Mat& org_mat, const std::list<cv::Mat>& mat_histories)
{
	return wait_nxt_stable(org_mat, mat_histories);
}

bool player::wait_nxt_stable(const cv::Mat& org_mat, const std::list<cv::Mat>& mat_histories)
{
	for (auto& nxt_child_cells : nxt_cells_)
	{
		for (auto& nxt_cell : nxt_child_cells)
		{
			const auto& org_roi = org_mat(nxt_cell.recognize_rect);
			const auto history_roi = mat_histories.front()(nxt_cell.recognize_rect);

			// 対象領域のピクセル毎に二乗誤差を計算
			cv::Mat diff_mat;
			subtract(org_roi, history_roi, diff_mat);
			cv::Mat pow_mat;
			pow(diff_mat, 2, pow_mat);

			// 対象領域のピクセルで平均をとり、全チャネルで合計した値をセット
			const auto before_stabilized = nxt_cell.is_stabilized();
			auto mse_bgr = cv::mean(pow_mat);
			nxt_cell.set_mse(static_cast<int>(mse_bgr[r] + mse_bgr[g] + mse_bgr[b]));

			// 安定状態に遷移した場合、色を更新
			if (!before_stabilized && nxt_cell.is_stabilized())
			{
				auto mean_bgr = cv::mean(org_roi);
				const auto r_val = static_cast<int>(mean_bgr[r]);
				const auto g_val = static_cast<int>(mean_bgr[g]);
				const auto b_val = static_cast<int>(mean_bgr[b]);

				auto set_color = color::none;
				if (r_val > 150 && g_val < 80 && b_val < 80)
					set_color = color::r;
				else if (r_val < 110 && g_val > 180 && b_val < 100)
					set_color = color::g;
				else if (r_val < 90 && g_val < 130 && b_val > 170)
					set_color = color::b;
				else if (r_val > 210 && g_val > 180 && b_val < 150)
					set_color = color::y;
				else if (r_val > 130 && g_val < 100 && b_val > 170)
					set_color = color::p;
				else if (r_val > 180 && g_val > 180 && b_val > 180)
					set_color = color::jam;
				nxt_cell.set_recognize_color(set_color);

				const auto logger = spdlog::get(logger_main);
				SPDLOG_LOGGER_TRACE(logger, "r:{} g:{} b{}", 
					static_cast<int>(r_val),
					static_cast<int>(g_val),
					static_cast<int>(b_val)
				);
			}
		}
	}

	return false;
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

	// endの線を描画
	end_cell_.debug_render(debug_mat);

	// nxtのmse
	SPDLOG_LOGGER_TRACE(logger, "game mse1:{} mse2:{} mse3:{} mse4:{}",
		nxt_cells_[0][axis].get_mse(),
		nxt_cells_[0][child].get_mse(),
		nxt_cells_[1][axis].get_mse(),
		nxt_cells_[1][child].get_mse()
	);
}


