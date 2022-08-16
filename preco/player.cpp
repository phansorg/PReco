#include "player.h"

#include <opencv2/imgproc.hpp>

#include "settings.h"
#include "logger.h"

player::player(const int player_idx)
{
	player_idx_ = player_idx;

	// Rect�̐ݒ�ǂݍ���
	auto& json = settings::get_instance()->json;
	field_frame_rect_ = cv::Rect(
		json["player_field_x"][player_idx_].get<int>(),
		json["player_field_y"].get<int>(),
		json["player_field_w"].get<int>(),
		json["player_field_h"].get<int>()
	);

	draw_frame_rects_[0] = cv::Rect(
		json["player_draw1_x"][player_idx_].get<int>(),
		json["player_draw1_y"].get<int>(),
		field_frame_rect_.width / cols,
		field_frame_rect_.height / rows
	);
	draw_frame_rects_[1] = cv::Rect(draw_frame_rects_[0]);
	draw_frame_rects_[1].y += draw_frame_rects_[1].height;

	draw_frame_rects_[2] = cv::Rect(
		json["player_draw2_x"][player_idx_].get<int>(),
		json["player_draw2_y"].get<int>(),
		draw_frame_rects_[0].width * 4 / 5,
		draw_frame_rects_[0].height * 4 / 5
	);
	draw_frame_rects_[3] = cv::Rect(draw_frame_rects_[2]);
	draw_frame_rects_[3].y += draw_frame_rects_[3].height;

	// ���̑�Rect�̌v�Z
	for(int idx = 0; idx < draw_cells; idx++)
	{
		draw_cell_rects_[idx] = to_recognize_rect(draw_frame_rects_[idx]);

	}
	init_wait_character_selection_rect();
	init_wait_reset_rect();

	histories_size_ = json["game_histories_size"].get<int>();
	draw_mse_ring_buffer_ = ring_buffer(mse_init, histories_size_, draw_cells);
}

// ============================================================
// rect
// ============================================================
cv::Rect player::to_recognize_rect(const cv::Rect frame)
{
	return {
		frame.x + frame.width / 4,
		frame.y + frame.height / 4,
		frame.width / 2,
		frame.height / 2
	};
}

void player::init_wait_character_selection_rect()
{
	// 1P�Ֆʂ̏㔼���̈悪�S�ĐԂł����OK
	// 2P�Ֆʂ̉������̈悪�S�ė΂ł����OK
	const auto y = player_idx_ == p1 ? 
		field_frame_rect_.y : field_frame_rect_.y + field_frame_rect_.height / 2;
	wait_character_selection_rect_ = cv::Rect(
		field_frame_rect_.x,
		y,
		field_frame_rect_.width,
		field_frame_rect_.height / 2
	);
}

void player::init_wait_reset_rect()
{
	// �Ֆʂ̒����̈悪�S�č��ł����OK
	wait_reset_rect_ = cv::Rect(
		field_frame_rect_.x + field_frame_rect_.width / 2,
		field_frame_rect_.y + field_frame_rect_.height / 2,
		field_frame_rect_.width / 10,
		field_frame_rect_.height / 10
	);
}

// ============================================================
// game
// ============================================================

bool player::wait_character_selection(const cv::Mat& org_mat) const
{
	std::vector<cv::Mat> channels;
	cv::Point* pos = nullptr;

	const auto roi_mat = org_mat(wait_reset_rect_);
	split(roi_mat, channels);
	if (player_idx_ == p1)
	{
		// 1P�Ֆʂ̏㔼���̈悪�S�ĐԂł����OK
		if (!checkRange(channels[b], true, pos, 0, 100)) return false;
		if (!checkRange(channels[g], true, pos, 0, 100)) return false;
		if (!checkRange(channels[r], true, pos, 180, 255)) return false;
	}
	else
	{
		// 2P�Ֆʂ̉������̈悪�S�ė΂ł����OK
		if (!checkRange(channels[b], true, pos, 0, 100)) return false;
		if (!checkRange(channels[g], true, pos, 180, 255)) return false;
		if (!checkRange(channels[r], true, pos, 0, 100)) return false;
	}

	return true;
}

bool player::wait_game_start(const cv::Mat& org_mat) const
{
	// �Ֆʂ̒����̈悪�S�č��ł����OK
	cv::Point* pos = nullptr;
	if (const auto roi_mat = org_mat(wait_reset_rect_);
		!checkRange(roi_mat, true, pos, 0, 30)) return false;

	return true;
}

bool player::game(int cur_no, const cv::Mat& org_mat, const std::list<cv::Mat>& mat_histories)
{
	const auto logger = spdlog::get(logger_main);
	SPDLOG_LOGGER_TRACE(logger, "game No:{}", cur_no);

	return wait_draw_stable(org_mat, mat_histories);
}

bool player::wait_draw_stable(const cv::Mat& org_mat, const std::list<cv::Mat>& mat_histories)
{
	const auto logger = spdlog::get(logger_main);

	draw_mse_ring_buffer_.next_record();
	for (int idx = 0; idx < draw_cells; idx++)
	{
		const auto& org_roi = org_mat(draw_cell_rects_[idx]);
		const auto history_roi = mat_histories.front()(draw_cell_rects_[idx]);

		cv::Mat diff_mat;
		subtract(org_roi, history_roi, diff_mat);
		cv::Mat pow_mat;
		pow(diff_mat, 2, pow_mat);
		auto mean = cv::mean(pow_mat);

		draw_mse_ring_buffer_.set(static_cast<int>(mean[r] + mean[b] + mean[b]), idx);
	}

	SPDLOG_LOGGER_TRACE(logger, "game mse1:{} mse2:{} mse3:{} mse4:{}",
		draw_mse_ring_buffer_.get(0),
		draw_mse_ring_buffer_.get(1),
		draw_mse_ring_buffer_.get(2),
		draw_mse_ring_buffer_.get(3)
	);
	return false;
}

// ============================================================
// debug
// ============================================================

void player::debug_wait_init(const cv::Mat& debug_mat) const
{
	// field�̐���`��
	auto rect = field_frame_rect_;
	auto x1 = rect.x;
	auto y1 = rect.y;
	auto x2 = rect.x + rect.width;
	auto y2 = rect.y + rect.height;
	rectangle(debug_mat, cv::Point(x1, y1), cv::Point(x2, y2), cv::Scalar(0, 0, 255), 1);

	// draw�̐���`��
	for (const auto& draw_rect : draw_cell_rects_)
	{
		rect = draw_rect;
		x1 = rect.x;
		y1 = rect.y;
		x2 = rect.x + rect.width;
		y2 = rect.y + rect.height;
		rectangle(debug_mat, cv::Point(x1, y1), cv::Point(x2, y2), cv::Scalar(0, 0, 255), 1);
	}
}
