#pragma once

#include <opencv2/core/types.hpp>

#include "ring_buffer.h"

enum class CellType
{
	kNone = 0,
	kBlock = 1, // ぷよ全般
};

enum class color {
	kR = 0,
	kG = 1,
	kB = 2,
	kY = 3,
	kP = 4,
	kJam = 5,
	kNone = 6,
};

class Cell
{
public:
	static constexpr int b_ = 0;
	static constexpr int g_ = 1;
	static constexpr int r_ = 2;

	static constexpr int h_ = 0;
	static constexpr int s_ = 1;
	static constexpr int v_ = 2;

	Cell();

	CellType type_;

	int row_;
	int col_;

	cv::Rect frame_rect_;
	cv::Rect recognize_rect_;

	color game_color_;

	int stabilized_threshold_count_ = 3;

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
	const int mse_threshold_ = 225;

	color recognize_color_;
	RingBuffer mse_ring_buffer_;

	int stabilize_count_;

};
