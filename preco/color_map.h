#pragma once
#include <map>

#include "cell.h"

class color_map
{
	std::map<color, unsigned char> map_;

public:
	void reset();
	void insert(color from_color);
	[[nodiscard]] unsigned char to_key(color from_color) const;

};

