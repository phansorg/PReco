#pragma once

#include <opencv2/core/types.hpp>

#include "ring_buffer.h"

class cell
{
public:
	cell();

	cv::Rect frame_rect;
	cv::Rect recognize_rect;

	void set_rect(cv::Rect in_rect);

	void set_mse(int mse);
	[[nodiscard]] int get_mse() const;

	[[nodiscard]] bool is_stabilizing() const;
	[[nodiscard]] bool is_stabilized() const;

	void debug_render(const cv::Mat& debug_mat) const;
	void render_rect(const cv::Mat& debug_mat, cv::Rect rect, const cv::Scalar& color) const;

private:
	ring_buffer mse_ring_buffer_;

	int stabilize_count_;

};
