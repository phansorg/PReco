#include "player.h"

#include <opencv2/imgproc.hpp>

#include "settings.h"
#include "logger.h"

player::player(const int player_idx)
{
	player_idx_ = player_idx;

	// field
	auto& json = settings::get_instance()->json;
	field_frame_rect_ = cv::Rect(
		json["player_field_x"][player_idx_].get<int>(),
		json["player_field_y"].get<int>(),
		json["player_field_w"].get<int>(),
		json["player_field_h"].get<int>()
	);
	cell_width_ = field_frame_rect_.width / cols;
	cell_height_ = field_frame_rect_.height / rows;

	// nxt1
	nxt_cells_[0].set_rect(cv::Rect(
		json["player_nxt1_x"][player_idx_].get<int>(),
		json["player_nxt1_y"].get<int>(),
		cell_width_,
		cell_height_
	));
	auto clone_rect = nxt_cells_[0].rect;
	clone_rect.y += clone_rect.height;
	nxt_cells_[1].set_rect(clone_rect);

	// nxt2
	nxt_cells_[2].set_rect(cv::Rect(
		json["player_nxt2_x"][player_idx_].get<int>(),
		json["player_nxt2_y"].get<int>(),
		cell_width_ * 4 / 5,
		cell_height_ * 4 / 5
	));
	clone_rect = nxt_cells_[2].rect;
	clone_rect.y += clone_rect.height;
	nxt_cells_[3].set_rect(clone_rect);

	// histories
	histories_max_ = json["game_histories_max"].get<int>();
	nxt_mse_ring_buffer_ = ring_buffer(mse_init, histories_max_, nxt_count);

	// ‚»‚Ì‘¼Rect‚ÌŒvZ
	init_field_cells();
	init_wait_character_selection_rect();
	init_wait_reset_rect();
	init_wait_end_rect();
}

// ============================================================
// rect
// ============================================================
void player::init_field_cells()
{
	const auto width = field_frame_rect_.width;
	const auto height = field_frame_rect_.height;
	for (int row = 0; row < rows; row++)
	{
		const auto y1 = height * row / rows + field_frame_rect_.y;
		const auto y2 = height * (row + 1) / rows + field_frame_rect_.y;
		for (int col = 0; col < cols; col++)
		{
			const auto x1 = width * col / cols + field_frame_rect_.x;
			const auto x2 = width * (col + 1) / cols + field_frame_rect_.x;
			field_cells_[row][col].set_rect(cv::Rect(
				x1,
				y1,
				x2 - x1,
				y2 - y1
			));
		}
	}
}

void player::init_wait_character_selection_rect()
{
	// 1P”Õ–Ê‚Ìã”¼•ª—Ìˆæ‚ª‘S‚ÄÔ‚Å‚ ‚ê‚ÎOK
	// 2P”Õ–Ê‚Ì‰º”¼•ª—Ìˆæ‚ª‘S‚Ä—Î‚Å‚ ‚ê‚ÎOK
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
	// ”Õ–Ê‚Ì’†‰›—Ìˆæ‚ª‘S‚Ä•‚Å‚ ‚ê‚ÎOK
	wait_reset_rect_ = cv::Rect(
		field_frame_rect_.x + field_frame_rect_.width / 2,
		field_frame_rect_.y + field_frame_rect_.height / 2,
		cell_width_,
		cell_height_
	);
}

void player::init_wait_end_rect()
{
	// Šp‚Ì—Ìˆæ‚ª‘S‚Ä—Î‚Å‚ ‚ê‚ÎOK
	const auto x = player_idx_ == p1 ?
		field_frame_rect_.x : field_frame_rect_.x + field_frame_rect_.width - 5;
	wait_end_rect_ = cv::Rect(
		x,
		field_frame_rect_.y + field_frame_rect_.height + 5,
		5,
		5
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
		// 1P”Õ–Ê‚Ìã”¼•ª—Ìˆæ‚ª‘S‚ÄÔ‚Å‚ ‚ê‚ÎOK
		if (!checkRange(channels[b], true, pos, 0, 100)) return false;
		if (!checkRange(channels[g], true, pos, 0, 100)) return false;
		if (!checkRange(channels[r], true, pos, 180, 255)) return false;
	}
	else
	{
		// 2P”Õ–Ê‚Ì‰º”¼•ª—Ìˆæ‚ª‘S‚Ä—Î‚Å‚ ‚ê‚ÎOK
		if (!checkRange(channels[b], true, pos, 0, 100)) return false;
		if (!checkRange(channels[g], true, pos, 180, 255)) return false;
		if (!checkRange(channels[r], true, pos, 0, 100)) return false;
	}

	return true;
}

bool player::wait_game_start(const cv::Mat& org_mat) const
{
	// ”Õ–Ê‚Ì’†‰›—Ìˆæ‚ª‘S‚Ä•‚Å‚ ‚ê‚ÎOK
	cv::Point* pos = nullptr;
	if (const auto roi_mat = org_mat(wait_reset_rect_);
		!checkRange(roi_mat, true, pos, 0, 30)) return false;

	return true;
}

bool player::game(int cur_no, const cv::Mat& org_mat, const std::list<cv::Mat>& mat_histories)
{
	const auto logger = spdlog::get(logger_main);
	SPDLOG_LOGGER_TRACE(logger, "game No:{}", cur_no);

	return wait_nxt_stable(org_mat, mat_histories);
}

bool player::wait_nxt_stable(const cv::Mat& org_mat, const std::list<cv::Mat>& mat_histories)
{
	const auto logger = spdlog::get(logger_main);

	nxt_mse_ring_buffer_.next_record();
	for (int idx = 0; idx < nxt_count; idx++)
	{
		const auto& org_roi = org_mat(nxt_cells_[idx].recognize_rect);
		const auto history_roi = mat_histories.front()(nxt_cells_[idx].recognize_rect);

		cv::Mat diff_mat;
		subtract(org_roi, history_roi, diff_mat);
		cv::Mat pow_mat;
		pow(diff_mat, 2, pow_mat);
		auto mean = cv::mean(pow_mat);

		nxt_mse_ring_buffer_.set(static_cast<int>(mean[r] + mean[b] + mean[b]), idx);
	}

	SPDLOG_LOGGER_TRACE(logger, "game mse1:{} mse2:{} mse3:{} mse4:{}",
		nxt_mse_ring_buffer_.get(0),
		nxt_mse_ring_buffer_.get(1),
		nxt_mse_ring_buffer_.get(2),
		nxt_mse_ring_buffer_.get(3)
	);
	return false;
}

// ============================================================
// debug
// ============================================================

void player::debug_render(const cv::Mat& debug_mat) const
{
	// field‚Ìü‚ğ•`‰æ
	render_rect(debug_mat, field_frame_rect_, cv::Scalar(0, 0, 255));
	for (const auto& field_row : field_cells_)
	{
		for (const auto& field_cell : field_row)
		{
			render_rect(debug_mat, field_cell.rect, cv::Scalar(0, 0, 255));
			render_rect(debug_mat, field_cell.recognize_rect, cv::Scalar(0, 0, 255));
		}
	}
	// nxt‚Ìü‚ğ•`‰æ
	for (const auto& nxt_cell : nxt_cells_)
	{
		render_rect(debug_mat, nxt_cell.rect, cv::Scalar(0, 0, 255));
		render_rect(debug_mat, nxt_cell.recognize_rect, cv::Scalar(0, 0, 255));
	}

	// end‚Ìü‚ğ•`‰æ
	render_rect(debug_mat, wait_end_rect_, cv::Scalar(0, 0, 0));
}

void player::render_rect(const cv::Mat& debug_mat, const cv::Rect rect, const cv::Scalar& color) const
{
	const auto x1 = rect.x;
	const auto y1 = rect.y;
	const auto x2 = rect.x + rect.width - 1;
	const auto y2 = rect.y + rect.height - 1;
	rectangle(debug_mat, cv::Point(x1, y1), cv::Point(x2, y2), color, 1);
}
