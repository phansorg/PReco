#include "player.h"

#include <opencv2/imgproc.hpp>

#include "settings.h"

player::player(const int player_idx)
{
	player_idx_ = player_idx;

	auto& json = settings::get_instance()->json;
	field_frame_rect_ = cv::Rect(
		json["player_field_x"][player_idx_].get<int>(),
		json["player_field_y"].get<int>(),
		json["player_field_w"].get<int>(),
		json["player_field_h"].get<int>()
	);

	draw_frame_rects_[0] = cv::Rect(
		json["player_draw1_x"][player_idx_].get<int>(),
		json["player_draw1_y"].get<int>(),
		field_frame_rect_.width / cols,
		field_frame_rect_.height / rows
	);
	draw_frame_rects_[1] = cv::Rect(draw_frame_rects_[0]);
	draw_frame_rects_[1].y += draw_frame_rects_[1].height;

	draw_frame_rects_[2] = cv::Rect(
		json["player_draw2_x"][player_idx_].get<int>(),
		json["player_draw2_y"].get<int>(),
		draw_frame_rects_[0].width * 4 / 5,
		draw_frame_rects_[0].height * 4 / 5
	);
	draw_frame_rects_[3] = cv::Rect(draw_frame_rects_[2]);
	draw_frame_rects_[3].y += draw_frame_rects_[3].height;

	for(int idx = 0; idx < draw_cells; idx++)
	{
		draw_cell_rects_[idx] = to_recognize_rect(draw_frame_rects_[idx]);

	}

	init_wait_character_selection_rect();
	init_wait_reset_rect();
}

// ============================================================
// rect
// ============================================================
cv::Rect player::to_recognize_rect(const cv::Rect frame)
{
	return {
		frame.x + frame.width / 4,
		frame.y + frame.height / 4,
		frame.width / 2,
		frame.height / 2
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
		field_frame_rect_.width / 10,
		field_frame_rect_.height / 10
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

// ============================================================
// debug
// ============================================================

void player::debug_wait_init(const cv::Mat& debug_mat) const
{
	// fieldの線を描画
	auto rect = field_frame_rect_;
	auto x1 = rect.x;
	auto y1 = rect.y;
	auto x2 = rect.x + rect.width;
	auto y2 = rect.y + rect.height;
	rectangle(debug_mat, cv::Point(x1, y1), cv::Point(x2, y2), cv::Scalar(0, 0, 255), 1);

	// drawの線を描画
	for (const auto& draw_rect : draw_cell_rects_)
	{
		rect = draw_rect;
		x1 = rect.x;
		y1 = rect.y;
		x2 = rect.x + rect.width;
		y2 = rect.y + rect.height;
		rectangle(debug_mat, cv::Point(x1, y1), cv::Point(x2, y2), cv::Scalar(0, 0, 255), 1);
	}
}
