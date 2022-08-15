#include "player.h"

#include "settings.h"

player::player(const int player_idx)
{
	player_idx_ = player_idx;

	auto& json = settings::get_instance()->json;
	field_x = json["recognize_field_x"].get<int>();

	if (player_idx == p2)
	{
		field_x += 651;
	}
}