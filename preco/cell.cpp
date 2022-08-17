#include "cell.h"

cell::cell()
{
	rect = cv::Rect(0, 0, 0, 0);
	recognize_rect = cv::Rect(0, 0, 0, 0);
}

void cell::set_rect(const cv::Rect in_rect)
{
	rect.x = in_rect.x;
	rect.y = in_rect.y;
	rect.width = in_rect.width;
	rect.height = in_rect.height;

	recognize_rect.x = rect.x + rect.width / 4;
	recognize_rect.y = rect.y + rect.height / 4;
	recognize_rect.width = rect.width / 2;
	recognize_rect.height = rect.height / 2;
}