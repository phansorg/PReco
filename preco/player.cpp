#include "player.h"

#include <opencv2/imgproc.hpp>

#include "settings.h"

player::player(const int player_idx)
{
	player_idx_ = player_idx;

	auto& json = settings::get_instance()->json;
	field_x_ = json["player_field_x"][player_idx_].get<int>();
	field_y_ = json["player_field_y"].get<int>();
	field_w_ = json["player_field_w"].get<int>();
	field_h_ = json["player_field_h"].get<int>();
	draw1_x_ = json["player_draw1_x"][player_idx_].get<int>();
	draw1_y_ = json["player_draw1_y"].get<int>();
	draw2_x_ = json["player_draw2_x"][player_idx_].get<int>();
	draw2_y_ = json["player_draw2_y"].get<int>();

	draw1_w_ = field_w_ / cols;
	draw1_h_ = field_h_ / rows;
	draw2_w_ = draw1_w_ * 4 / 5;
	draw2_h_ = draw1_h_ * 4 / 5;

	init_field_rect();
	init_draw_rect_vector();
	init_wait_character_selection_rect();
	init_wait_reset_rect();
}

// ============================================================
// rect
// ============================================================

void player::init_field_rect()
{
	field_rect_ = cv::Rect(
		field_x_,
		field_y_,
		field_w_,
		field_h_
	);
}

void player::init_draw_rect_vector()
{
	auto x = draw1_x_ + draw1_w_ / 4;
	auto y = draw1_y_ + draw1_h_ / 4;
	auto w = draw1_w_ / 2;
	auto h = draw1_h_ / 2;
	draw_rect_vector_.emplace_back(x, y, w, h);

	y += draw1_h_;
	draw_rect_vector_.emplace_back(x, y, w, h);

	x = draw2_x_ + draw2_w_ / 4;
	y = draw2_y_ + draw2_h_ / 4;
	w = draw2_w_ / 2;
	h = draw2_h_ / 2;
	draw_rect_vector_.emplace_back(x, y, w, h);

	y += draw2_h_;
	draw_rect_vector_.emplace_back(x, y, w, h);
}

void player::init_wait_character_selection_rect()
{
	// 1P盤面の上半分領域が全て赤であればOK
	// 2P盤面の下半分領域が全て緑であればOK
	const auto y = player_idx_ == p1 ? field_y_ : field_y_ + field_h_ / 2;
	wait_character_selection_rect_ = cv::Rect(
		field_x_,
		y,
		field_w_,
		field_h_ / 2
	);
}

void player::init_wait_reset_rect()
{
	// 盤面の中央領域が全て黒であればOK
	wait_reset_rect_ = cv::Rect(
		field_x_ + field_w_ / 2,
		field_y_ + field_h_ / 2,
		field_w_ / 10,
		field_h_ / 10
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
	auto rect = field_rect_;
	auto x1 = rect.x;
	auto y1 = rect.y;
	auto x2 = rect.x + rect.width;
	auto y2 = rect.y + rect.height;
	rectangle(debug_mat, cv::Point(x1, y1), cv::Point(x2, y2), cv::Scalar(0, 0, 255), 1);

	// drawの線を描画
	for (const auto& draw_rect : draw_rect_vector_)
	{
		rect = draw_rect;
		x1 = rect.x;
		y1 = rect.y;
		x2 = rect.x + rect.width;
		y2 = rect.y + rect.height;
		rectangle(debug_mat, cv::Point(x1, y1), cv::Point(x2, y2), cv::Scalar(0, 0, 255), 1);
	}
}
