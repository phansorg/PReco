#include "player.h"

#include <opencv2/imgproc.hpp>

#include "settings.h"
#include "logger.h"

player::player(const int player_idx)
{
	player_idx_ = player_idx;
	move_next_idx_ = 0;

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
	nxt_cells_[0][axis].set_rect(cv::Rect(
		json["player_nxt1_x"][player_idx_].get<int>(),
		json["player_nxt1_y"].get<int>(),
		cell_width_,
		cell_height_
	));
	auto clone_rect = nxt_cells_[0][axis].frame_rect;
	clone_rect.y += clone_rect.height;
	nxt_cells_[0][child].set_rect(clone_rect);

	// nxt2
	nxt_cells_[1][axis].set_rect(cv::Rect(
		json["player_nxt2_x"][player_idx_].get<int>(),
		json["player_nxt2_y"].get<int>(),
		cell_width_ * 4 / 5,
		cell_height_ * 4 / 5
	));
	clone_rect = nxt_cells_[1][axis].frame_rect;
	clone_rect.y += clone_rect.height;
	nxt_cells_[1][child].set_rect(clone_rect);

	// combo
	combo_cell_.set_rect(cv::Rect(
		json["player_score_x"][player_idx_].get<int>() + 4,
		json["player_score_y"].get<int>() + 1,
		cell_width_ / 2,
		cell_height_ / 4
	));

	// ���̑�Rect�̌v�Z
	init_field_cells();
	init_combo_cell();
	init_end_cell();
	init_wait_character_selection_rect();
	init_wait_reset_rect();
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

void player::init_combo_cell()
{
	// �X�R�A�̍��[���ω�������combo
	const auto width = cell_width_ / 2;
	const auto x = player_idx_ == p1 ?
		field_frame_rect_.x : field_frame_rect_.x + field_frame_rect_.width - width;
	end_cell_.set_rect(cv::Rect(
		x,
		field_frame_rect_.y + field_frame_rect_.height + width / 2,
		width,
		width
	));
}

void player::init_end_cell()
{
	// �p�̗̈悪�S�ė΂ł����end
	const auto width = cell_width_ / 2;
	const auto x = player_idx_ == p1 ?
		field_frame_rect_.x : field_frame_rect_.x + field_frame_rect_.width - width;
	end_cell_.set_rect(cv::Rect(
		x,
		field_frame_rect_.y + field_frame_rect_.height + width / 2,
		width,
		width
	));
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
		cell_width_,
		cell_height_
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
		if (!checkRange(channels[cell::b], true, pos, 0, 100)) return false;
		if (!checkRange(channels[cell::g], true, pos, 0, 100)) return false;
		if (!checkRange(channels[cell::r], true, pos, 180, 255)) return false;
	}
	else
	{
		// 2P�Ֆʂ̉������̈悪�S�ė΂ł����OK
		if (!checkRange(channels[cell::b], true, pos, 0, 100)) return false;
		if (!checkRange(channels[cell::g], true, pos, 180, 255)) return false;
		if (!checkRange(channels[cell::r], true, pos, 0, 100)) return false;
	}

	return true;
}

bool player::wait_game_reset(const cv::Mat& org_mat)
{
	// �Ֆʂ̒����̈悪�S�č��ł����OK
	cv::Point* pos = nullptr;
	if (const auto roi_mat = org_mat(wait_reset_rect_);
		!checkRange(roi_mat, true, pos, 0, 30)) return false;

	// field�����Z�b�g
	for (auto& field_row : field_cells_)
	{
		for (auto& field_cell : field_row)
		{
			field_cell.reset();
		}
	}

	// nxt�����Z�b�g
	for (auto& nxt_child_cells : nxt_cells_)
	{
		for (auto& nxt_cell : nxt_child_cells)
		{
			nxt_cell.reset();
		}
	}

	return true;
}

bool player::wait_game_init(const cv::Mat& org_mat, const std::list<cv::Mat>& mat_histories)
{
	// nxt���S�Ĉ��肵�A�F�������OK
	update_nxt_cells(org_mat, mat_histories);
	for (auto& nxt_child_cells : nxt_cells_)
	{
		for (auto& nxt_cell : nxt_child_cells)
		{
			if (!nxt_cell.is_stabilized()) return false;
			if (nxt_cell.get_recognize_color() == color::none) return false;
		}
	}

	// ��������������nxt��o�^
	for (auto& nxt_child_cells : nxt_cells_)
	{
		for (auto& nxt_cell : nxt_child_cells)
		{
			nxt_records_.push_back(nxt_cell.get_recognize_color());
		}
	}
	move_next_idx_ = 0;

	return true;
}

bool player::game(int cur_no, const cv::Mat& org_mat, const std::list<cv::Mat>& mat_histories)
{
	update_all_cells(org_mat, mat_histories);
	return false;
}

void player::update_all_cells(const cv::Mat& org_mat, const std::list<cv::Mat>& mat_histories)
{
	// field
	for (auto& field_row : field_cells_)
	{
		for (auto& field_cell : field_row)
		{
			update_cell(org_mat, mat_histories, field_cell);
		}
	}

	// nxt
	update_nxt_cells(org_mat, mat_histories);

	// combo
	update_cell(org_mat, mat_histories, combo_cell_);

	// end
	update_cell(org_mat, mat_histories, end_cell_);
}

void player::update_nxt_cells(const cv::Mat& org_mat, const std::list<cv::Mat>& mat_histories)
{
	for (auto& nxt_child_cells : nxt_cells_)
	{
		for (auto& nxt_cell : nxt_child_cells)
		{
			update_cell(org_mat, mat_histories, nxt_cell);
		}
	}
}

void player::update_cell(const cv::Mat& org_mat, const std::list<cv::Mat>& mat_histories, cell& target_cell) const
{
	// �Ώۗ̈���擾���AHSV�ɕϊ�
	const auto& org_roi = org_mat(target_cell.recognize_rect);
	cv::Mat org_hsv_mat;
	cvtColor(org_roi, org_hsv_mat, cv::COLOR_BGR2HSV);

	const auto history_roi = mat_histories.front()(target_cell.recognize_rect);
	cv::Mat history_hsv_mat;
	cvtColor(history_roi, history_hsv_mat, cv::COLOR_BGR2HSV);

	// �Ώۗ̈�̃s�N�Z������MSE���v�Z
	cv::Mat diff_mat;
	subtract(org_hsv_mat, history_hsv_mat, diff_mat);
	cv::Mat pow_mat;
	pow(diff_mat, 2, pow_mat);

	// MSE�̕��ς��Ƃ�Acell�ɐݒ�
	const auto before_stabilized = target_cell.is_stabilized();
	target_cell.set_mse(mean(pow_mat));

	// �����ԂɑJ�ڂ����ꍇ�A�F���X�V
	if (!before_stabilized && target_cell.is_stabilized())
	{
		target_cell.update_recognize_color(mean(org_roi));
	}
}

// ============================================================
// debug
// ============================================================

void player::debug_render(const cv::Mat& debug_mat) const
{
	const auto logger = spdlog::get(logger_main);

	// field�̐���`��
	for (const auto& field_row : field_cells_)
	{
		for (const auto& field_cell : field_row)
		{
			field_cell.debug_render(debug_mat);
		}
	}
	// nxt�̐���`��
	for (const auto& nxt_child_cells : nxt_cells_)
	{
		for (const auto& nxt_cell : nxt_child_cells)
		{
			nxt_cell.debug_render(debug_mat);
		}
	}

	// combo�̐���`��
	combo_cell_.debug_render(debug_mat);

	// end�̐���`��
	end_cell_.debug_render(debug_mat);

	// nxt��mse
	SPDLOG_LOGGER_TRACE(logger, "game mse1:{} mse2:{} mse3:{} mse4:{}",
		nxt_cells_[0][axis].get_mse(),
		nxt_cells_[0][child].get_mse(),
		nxt_cells_[1][axis].get_mse(),
		nxt_cells_[1][child].get_mse()
	);
}


