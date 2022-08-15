#pragma once

class player
{
	int player_idx_;

public:
	explicit player(int player_idx);

	static constexpr int p1 = 0;
	static constexpr int p2 = 1;

	int field_x;
};
