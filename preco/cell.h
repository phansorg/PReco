#pragma once

#include <opencv2/core/types.hpp>

#include "ring_buffer.h"

enum class cell_type
{
	none = 0,
	block = 1,
};

enum class color {
	r = 0,
	g = 1,
	b = 2,
	y = 3,
	p = 4,
	jam = 5,
	none = 6,
};

class cell
{
public:
	static constexpr int b = 0;
	static constexpr int g = 1;
	static constexpr int r = 2;

	static constexpr int h = 0;
	static constexpr int s = 1;
	static constexpr int v = 2;

	cell();

	cell_type type;

	int row;
	int col;

	cv::Rect frame_rect;
	cv::Rect recognize_rect;

	color game_color;

	void set_rect(cv::Rect in_rect);

	void reset();

	void update_recognize_color(const cv::Scalar& bgr_scalar);
	[[nodiscard]] color get_recognize_color() const;

	void set_mse(const cv::Scalar& hsv_scalar);
	[[nodiscard]] int get_mse() const;

	[[nodiscard]] bool is_stabilizing() const;
	[[nodiscard]] bool is_stabilized() const;

	void debug_render(const cv::Mat& debug_mat) const;
	void render_rect(const cv::Mat& debug_mat, cv::Rect rect, const cv::Scalar& bgr_scalar) const;

private:
	const int mse_threshold_ = 170;

	color recognize_color_;
	ring_buffer mse_ring_buffer_;

	int stabilize_count_;

};
