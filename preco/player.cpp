#include "player.h"

#include "settings.h"

player::player(const int player_idx)
{
	player_idx_ = player_idx;

	auto& json = settings::get_instance()->json;
	field_x_ = json["player_field_x"].get<int>();
	field_y_ = json["player_field_y"].get<int>();
	field_width_ = json["player_field_width"].get<int>();
	field_height_ = json["player_field_height"].get<int>();
	if (player_idx == p2)
	{
		field_x_ += 651;
	}

	init_field_rect();
	init_wait_character_rect();
	init_wait_reset_rect();
}

void player::init_field_rect()
{
	field_rect = cv::Rect(
		field_x_,
		field_y_,
		field_width_,
		field_height_
	);
}

void player::init_wait_character_rect()
{
	// 1P”Õ–Ê‚Ìã”¼•ª—Ìˆæ‚ª‘S‚ÄÔ‚Å‚ ‚ê‚ÎOK
	// 2P”Õ–Ê‚Ì‰º”¼•ª—Ìˆæ‚ª‘S‚Ä—Î‚Å‚ ‚ê‚ÎOK
	const auto y = player_idx_ == p1 ? field_y_ : field_y_ + field_height_ / 2;
	wait_character_rect = cv::Rect(
		field_x_,
		y,
		field_width_,
		field_height_ / 2
	);
}

void player::init_wait_reset_rect()
{
	// ”Õ–Ê‚Ì’†‰›—Ìˆæ‚ª‘S‚Ä•‚Å‚ ‚ê‚ÎOK
	wait_reset_rect = cv::Rect(
		field_x_ + field_width_ / 2,
		field_y_ + field_height_ / 2,
		field_width_ / 10,
		field_height_ / 10
	);
}


