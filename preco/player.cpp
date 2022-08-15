#include "player.h"

#include "settings.h"

player::player(const int player_idx)
{
	player_idx_ = player_idx;

	auto& json = settings::get_instance()->json;
	field_x_ = json["player_field_x"][player_idx_].get<int>();
	field_y_ = json["player_field_y"].get<int>();
	field_w_ = json["player_field_w"].get<int>();
	field_h_ = json["player_field_h"].get<int>();
	next1_x_ = json["player_next1_x"][player_idx_].get<int>();
	next1_y_ = json["player_next1_y"].get<int>();
	next2_x_ = json["player_next2_x"][player_idx_].get<int>();
	next2_y_ = json["player_next2_y"].get<int>();

	next1_w_ = field_w_ / cols;
	next1_h_ = field_h_ / rows;
	next2_w_ = next1_w_ * 4 / 5;
	next2_h_ = next1_h_ * 4 / 5;

	init_field_rect();
	init_next_rect_vector();
	init_wait_character_rect();
	init_wait_reset_rect();
}

void player::init_field_rect()
{
	field_rect = cv::Rect(
		field_x_,
		field_y_,
		field_w_,
		field_h_
	);
}

void player::init_next_rect_vector()
{
	auto x = next1_x_ + next1_w_ / 4;
	auto y = next1_y_ + next1_h_ / 4;
	auto w = next1_w_ / 2;
	auto h = next1_h_ / 2;
	next_rect_vector.emplace_back(x, y, w, h);

	y += next1_h_;
	next_rect_vector.emplace_back(x, y, w, h);

	x = next2_x_ + next2_w_ / 4;
	y = next2_y_ + next2_h_ / 4;
	w = next2_w_ / 2;
	h = next2_h_ / 2;
	next_rect_vector.emplace_back(x, y, w, h);

	y += next2_h_;
	next_rect_vector.emplace_back(x, y, w, h);
}

void player::init_wait_character_rect()
{
	// 1P”Õ–Ê‚Ìã”¼•ª—Ìˆæ‚ª‘S‚ÄÔ‚Å‚ ‚ê‚ÎOK
	// 2P”Õ–Ê‚Ì‰º”¼•ª—Ìˆæ‚ª‘S‚Ä—Î‚Å‚ ‚ê‚ÎOK
	const auto y = player_idx_ == p1 ? field_y_ : field_y_ + field_h_ / 2;
	wait_character_rect = cv::Rect(
		field_x_,
		y,
		field_w_,
		field_h_ / 2
	);
}

void player::init_wait_reset_rect()
{
	// ”Õ–Ê‚Ì’†‰›—Ìˆæ‚ª‘S‚Ä•‚Å‚ ‚ê‚ÎOK
	wait_reset_rect = cv::Rect(
		field_x_ + field_w_ / 2,
		field_y_ + field_h_ / 2,
		field_w_ / 10,
		field_h_ / 10
	);
}


