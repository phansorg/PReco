#include "player.h"

#include <opencv2/imgproc.hpp>

#include "settings.h"
#include "logger.h"

player::player(const int player_idx)
{
	player_idx_ = player_idx;

	// Rectの設定読み込み
	auto& json = settings::get_instance()->json;
	field_frame_rect_ = cv::Rect(
		json["player_field_x"][player_idx_].get<int>(),
		json["player_field_y"].get<int>(),
		json["player_field_w"].get<int>(),
		json["player_field_h"].get<int>()
	);
	cell_width_ = field_frame_rect_.width / cols;
	cell_height_ = field_frame_rect_.height / rows;

	nxt_cell_rects_[0] = cv::Rect(
		json["player_nxt1_x"][player_idx_].get<int>(),
		json["player_nxt1_y"].get<int>(),
		cell_width_,
		cell_height_
	);
	nxt_cell_rects_[1] = cv::Rect(nxt_cell_rects_[0]);
	nxt_cell_rects_[1].y += nxt_cell_rects_[1].height;

	nxt_cell_rects_[2] = cv::Rect(
		json["player_nxt2_x"][player_idx_].get<int>(),
		json["player_nxt2_y"].get<int>(),
		cell_width_ * 4 / 5,
		cell_height_ * 4 / 5
	);
	nxt_cell_rects_[3] = cv::Rect(nxt_cell_rects_[2]);
	nxt_cell_rects_[3].y += nxt_cell_rects_[3].height;

	// その他Rectの計算
	for(int idx = 0; idx < nxt_cells; idx++)
	{
		nxt_recognize_rects_[idx] = to_recognize_rect(nxt_cell_rects_[idx]);

	}
	init_wait_character_selection_rect();
	init_wait_reset_rect();
	init_wait_end_rect();

	histories_size_ = json["game_histories_size"].get<int>();
	nxt_mse_ring_buffer_ = ring_buffer(mse_init, histories_size_, nxt_cells);
}

// ============================================================
// rect
// ============================================================
cv::Rect player::to_recognize_rect(const cv::Rect cell)
{
	return {
		cell.x + cell.width / 4,
		cell.y + cell.height / 4,
		cell.width / 2,
		cell.height / 2
	};
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
	const auto x = player_idx_ == p1 ?
		field_frame_rect_.x : field_frame_rect_.x + field_frame_rect_.width - 5;
	wait_end_rect_ = cv::Rect(
		x,
		field_frame_rect_.y + field_frame_rect_.height + 5,
		5,
		5
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
	const auto logger = spdlog::get(logger_main);
	SPDLOG_LOGGER_TRACE(logger, "game No:{}", cur_no);

	return wait_nxt_stable(org_mat, mat_histories);
}

bool player::wait_nxt_stable(const cv::Mat& org_mat, const std::list<cv::Mat>& mat_histories)
{
	const auto logger = spdlog::get(logger_main);

	nxt_mse_ring_buffer_.next_record();
	for (int idx = 0; idx < nxt_cells; idx++)
	{
		const auto& org_roi = org_mat(nxt_recognize_rects_[idx]);
		const auto history_roi = mat_histories.front()(nxt_recognize_rects_[idx]);

		cv::Mat diff_mat;
		subtract(org_roi, history_roi, diff_mat);
		cv::Mat pow_mat;
		pow(diff_mat, 2, pow_mat);
		auto mean = cv::mean(pow_mat);

		nxt_mse_ring_buffer_.set(static_cast<int>(mean[r] + mean[b] + mean[b]), idx);
	}

	SPDLOG_LOGGER_TRACE(logger, "game mse1:{} mse2:{} mse3:{} mse4:{}",
		nxt_mse_ring_buffer_.get(0),
		nxt_mse_ring_buffer_.get(1),
		nxt_mse_ring_buffer_.get(2),
		nxt_mse_ring_buffer_.get(3)
	);
	return false;
}

// ============================================================
// debug
// ============================================================

void player::debug_render(const cv::Mat& debug_mat) const
{
	// fieldの線を描画
	auto rect = field_frame_rect_;
	auto x1 = rect.x;
	auto y1 = rect.y;
	auto x2 = rect.x + rect.width;
	auto y2 = rect.y + rect.height;
	rectangle(debug_mat, cv::Point(x1, y1), cv::Point(x2, y2), cv::Scalar(0, 0, 255), 1);

	// nxtの線を描画
	for (const auto& nxt_rect : nxt_recognize_rects_)
	{
		rect = nxt_rect;
		x1 = rect.x;
		y1 = rect.y;
		x2 = rect.x + rect.width;
		y2 = rect.y + rect.height;
		rectangle(debug_mat, cv::Point(x1, y1), cv::Point(x2, y2), cv::Scalar(0, 0, 255), 1);
	}

	// endの線を描画
	rect = wait_end_rect_;
	x1 = rect.x;
	y1 = rect.y;
	x2 = rect.x + rect.width;
	y2 = rect.y + rect.height;
	rectangle(debug_mat, cv::Point(x1, y1), cv::Point(x2, y2), cv::Scalar(0, 0, 0), 1);

	

}
