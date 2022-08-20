﻿#include "cell.h"

#include <opencv2/imgproc.hpp>

#include "settings.h"



cell::cell()
{
	frame_rect = cv::Rect(0, 0, 0, 0);
	recognize_rect = cv::Rect(0, 0, 0, 0);

	recognize_color_ = color::none;
	mse_ring_buffer_ = ring_buffer(settings::mse_init, settings::history_max);
	stabilize_count_ = 0;

}

void cell::set_rect(const cv::Rect in_rect)
{
	frame_rect.x = in_rect.x;
	frame_rect.y = in_rect.y;
	frame_rect.width = in_rect.width;
	frame_rect.height = in_rect.height;

	recognize_rect.x = frame_rect.x + frame_rect.width / 4;
	recognize_rect.y = frame_rect.y + frame_rect.height / 4;
	recognize_rect.width = frame_rect.width / 2;
	recognize_rect.height = frame_rect.height / 2;
}

void cell::set_recognize_color(const color recognize_color)
{
	recognize_color_ = recognize_color;
}

void cell::set_mse(const int mse)
{
	mse_ring_buffer_.next_record();
	mse_ring_buffer_.set(mse);
	
	if (mse >= mse_threshold_)
	{
		stabilize_count_ = 0;
		return;
	}

	stabilize_count_++;
}

int cell::get_mse() const
{
	return mse_ring_buffer_.get();
}

bool cell::is_stabilizing() const
{
	return stabilize_count_ >= 1;
}

bool cell::is_stabilized() const
{
	return stabilize_count_ >= 3;
}

void cell::debug_render(const cv::Mat& debug_mat) const
{
	// frame_rect
	cv::Scalar bgr_scalar;
	switch (recognize_color_)
	{
	case color::none:
		bgr_scalar = cv::Scalar(0, 0, 0);
		break;
	case color::r:
		bgr_scalar = cv::Scalar(0, 0, 255);
		break;
	case color::g:
		bgr_scalar = cv::Scalar(0, 255, 0);
		break;
	case color::b:
		bgr_scalar = cv::Scalar(255, 0, 0);
		break;
	case color::y:
		bgr_scalar = cv::Scalar(0, 255, 255);
		break;
	case color::p:
		bgr_scalar = cv::Scalar(255, 0, 255);
		break;
	case color::jam:
		bgr_scalar = cv::Scalar(255, 255, 255);
		break;
	}
	render_rect(debug_mat, frame_rect, bgr_scalar);

	// recognize_rect
	if (is_stabilized())
		bgr_scalar = cv::Scalar(0, 0, 0);
	else if (is_stabilizing())
		bgr_scalar = cv::Scalar(0, 255, 0);
	else
		bgr_scalar = cv::Scalar(0, 0, 255);
	render_rect(debug_mat, recognize_rect, bgr_scalar);
}
void cell::render_rect(const cv::Mat& debug_mat, const cv::Rect rect, const cv::Scalar& bgr_scalar) const
{
	const auto x1 = rect.x;
	const auto y1 = rect.y;
	const auto x2 = rect.x + rect.width - 1;
	const auto y2 = rect.y + rect.height - 1;
	rectangle(debug_mat, cv::Point(x1, y1), cv::Point(x2, y2), bgr_scalar, 2);
}
