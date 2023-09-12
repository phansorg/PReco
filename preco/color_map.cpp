#include "color_map.h"

void ColorMap::reset()
{
	map_.clear();
}

void ColorMap::insert(const color from_color)
{
	if (map_.contains(from_color)) return;

	unsigned char to_key = '1' + map_.size();
	map_.insert(std::make_pair(from_color, to_key));
}

unsigned char ColorMap::to_key(const color from_color) const
{
	return map_.at(from_color);
}
