#pragma once

#include <opencv2/core/types.hpp>

#include "ring_buffer.h"

enum class color {
	none = 0,
	r = 1,
	g = 2,
	b = 3,
	y = 4,
	p = 5,
	jam = 6,
};

class cell
{
public:
	cell();

	cv::Rect frame_rect;
	cv::Rect recognize_rect;

	void set_rect(cv::Rect in_rect);

	void set_recognize_color(color color);

	void set_mse(int mse);
	[[nodiscard]] int get_mse() const;

	[[nodiscard]] bool is_stabilizing() const;
	[[nodiscard]] bool is_stabilized() const;

	void debug_render(const cv::Mat& debug_mat) const;
	void render_rect(const cv::Mat& debug_mat, cv::Rect rect, const cv::Scalar& bgr_scalar) const;

private:
	const int mse_threshold_ = 180;

	color recognize_color_;
	ring_buffer mse_ring_buffer_;

	int stabilize_count_;

};
