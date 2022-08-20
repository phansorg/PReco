#include "cell.h"

#include "settings.h"

cell::cell()
{
	rect = cv::Rect(0, 0, 0, 0);
	recognize_rect = cv::Rect(0, 0, 0, 0);

	mse_ring_buffer_ = ring_buffer(settings::mse_init, settings::history_max);

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

void cell::set_mse(const int mse)
{
	mse_ring_buffer_.next_record();
	mse_ring_buffer_.set(mse);
}

int cell::get_mse() const
{
	return mse_ring_buffer_.get();
}

