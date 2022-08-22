#include "player.h"

#include <opencv2/imgproc.hpp>
#include <iostream>
#include <fstream>

#include "settings.h"
#include "logger.h"

player::player(const int player_idx)
{
	player_idx_ = player_idx;
	player_mode_ = player_mode::wait_game_start;
	cur_nxt_idx_ = -2;

	auto& json = settings::get_instance()->json;
	history_dir_ = json["player_history_dir"].get<std::string>();

	// field
	field_frame_rect_ = cv::Rect(
		json["player_field_x"][player_idx_].get<int>(),
		json["player_field_y"].get<int>(),
		json["player_field_w"].get<int>(),
		json["player_field_h"].get<int>()
	);
	cell_width_ = field_frame_rect_.width / cols;
	cell_height_ = field_frame_rect_.height / rows0;

	// nxt1
	auto row = 0;
	auto col = axis;
	nxt_cells_[row][col].row = row;
	nxt_cells_[row][col].col = col;
	nxt_cells_[row][col].set_rect(cv::Rect(
		json["player_nxt1_x"][player_idx_].get<int>(),
		json["player_nxt1_y"].get<int>(),
		cell_width_,
		cell_height_
	));

	auto clone_rect = nxt_cells_[row][col].frame_rect;
	col = child;
	clone_rect.y += clone_rect.height;
	nxt_cells_[row][col].row = row;
	nxt_cells_[row][col].col = col;
	nxt_cells_[row][col].set_rect(clone_rect);

	// nxt2
	row = 1;
	col = axis;
	nxt_cells_[row][col].row = row;
	nxt_cells_[row][col].col = col;
	nxt_cells_[row][col].set_rect(cv::Rect(
		json["player_nxt2_x"][player_idx_].get<int>(),
		json["player_nxt2_y"].get<int>(),
		cell_width_ * 4 / 5,
		cell_height_ * 4 / 5
	));
	clone_rect = nxt_cells_[row][col].frame_rect;
	col = child;
	clone_rect.y += clone_rect.height;
	nxt_cells_[row][col].row = 1;
	nxt_cells_[row][col].col = col;
	nxt_cells_[row][col].set_rect(clone_rect);

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
	for (int row = 0; row < rows2; row++)
	{
		const auto y_idx = rows2 - row - 3;
		const auto y1 = height * y_idx / rows0 + field_frame_rect_.y;
		const auto y2 = height * (y_idx + 1) / rows0 + field_frame_rect_.y;
		for (int col = 0; col < cols; col++)
		{
			const auto x1 = width * col / cols + field_frame_rect_.x;
			const auto x2 = width * (col + 1) / cols + field_frame_rect_.x;
			field_cells_[row][col].row = row;
			field_cells_[row][col].col = col;
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
	nxt_records_.clear();
	cur_nxt_idx_ = -2;

	// combo�����Z�b�g
	combo_cell_.reset();

	// end�����Z�b�g
	end_cell_.reset();

	return true;
}

bool player::wait_game_init(const cv::Mat& org_mat, const std::list<cv::Mat>& mat_histories)
{
	const auto logger = spdlog::get(logger_main);

	// �������ς݂̏ꍇ�AOK
	if (!nxt_records_.empty())
	{
		SPDLOG_LOGGER_TRACE(logger, "wait_game_init ok p:{}", player_idx_);
		return true;
	}

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

	// ��������������nxt��o�^���A�ēx���Z�b�g(game_start�ōēx�F�������邽��)
	for (auto& nxt_child_cells : nxt_cells_)
	{
		for (auto& nxt_cell : nxt_child_cells)
		{
			nxt_records_.push_back(nxt_cell.get_recognize_color());
			nxt_cell.reset();
		}
	}

	// history�̃t�@�C������ݒ�
	const std::time_t raw_time = time(nullptr);
	std::tm time_info{};
	char buffer[80];

	if (localtime_s(&time_info ,&raw_time))
	{
		logger->critical("localtime_s fail");
	}
	if (!std::strftime(buffer, 80, "%Y%m%d_%H%M%S", &time_info))
	{
		logger->critical("str format time fail");
	}

	std::ostringstream file_name;
	file_name << buffer << "_" << player_idx_ << ".txt";

	// �f�B���N�g���p�X�Əo�̓t�@�C����������
	history_path_ = history_dir_;
	history_path_.append(file_name.str());

	// player_mode���Q�[���J�n��ɃZ�b�g
	player_mode_ = player_mode::wait_nxt_stabilize;
	logger->info("p:{} player_mode:{}", player_idx_, static_cast<int>(player_mode_));

	return true;
}

bool player::game(int cur_no, const cv::Mat& org_mat, const std::list<cv::Mat>& mat_histories)
{
	update_all_cells(org_mat, mat_histories);

	switch (player_mode_)
	{
	case player_mode::wait_game_start:
		break;
	case player_mode::wait_nxt_stabilize:
		wait_nxt_stabilize();
		break;
	case player_mode::wait_nxt_change:
		wait_nxt_change();
		break;
	}

	return wait_game_end();
}

void player::wait_nxt_stabilize()
{
	for (auto& nxt_child_cells : nxt_cells_)
	{
		for (auto& nxt_cell : nxt_child_cells)
		{
			if (!nxt_cell.is_stabilized()) return;
		}
	}

	nxt_records_.push_back(nxt_cells_[1][axis].get_recognize_color());
	nxt_records_.push_back(nxt_cells_[1][child].get_recognize_color());
	cur_nxt_idx_ += 2;

	const auto logger = spdlog::get(logger_main);
	player_mode_ = player_mode::wait_nxt_change;
	logger->info("p:{} player_mode:{}", player_idx_, static_cast<int>(player_mode_));
}

void player::wait_nxt_change()
{
	for (auto& nxt_child_cells : nxt_cells_)
	{
		for (auto& nxt_cell : nxt_child_cells)
		{
			if (nxt_cell.is_stabilized()) return;
		}
	}

	write_history();

	const auto logger = spdlog::get(logger_main);
	player_mode_ = player_mode::wait_nxt_stabilize;
	logger->info("p:{} player_mode:{}", player_idx_, static_cast<int>(player_mode_));
}

bool player::wait_game_end() const
{
	return end_cell_.get_recognize_color() == color::g;
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

void player::game_end()
{
	const auto logger = spdlog::get(logger_main);
	player_mode_ = player_mode::wait_game_start;
	logger->info("p:{} player_mode:{}", player_idx_, static_cast<int>(player_mode_));
}

void player::write_history() const
{
	std::ofstream history_file(history_path_, std::ios::app);
	if (!history_file)
	{
		return;
	}

	unsigned char field_text[rows2 * cols + 1] = {0};
	to_field_text(field_text);

	history_file
		<< to_color_text(nxt_records_[cur_nxt_idx_])
		<< to_color_text(nxt_records_[cur_nxt_idx_ + 1])
		<< ","
		<< to_color_text(nxt_records_[cur_nxt_idx_ + 2])
		<< to_color_text(nxt_records_[cur_nxt_idx_ + 3])
		<< ","
		<< to_color_text(nxt_records_[cur_nxt_idx_ + 4])
		<< to_color_text(nxt_records_[cur_nxt_idx_ + 5])
		<< ","
	    << field_text
		<< std::endl;
	history_file.close();
}

unsigned char player::to_color_text(const color from_color)
{
	switch (from_color)
	{
	case color::r: return 'R';
	case color::g: return 'G';
	case color::b: return 'B';
	case color::y: return 'Y';
	case color::p: return 'P';
	case color::jam: return 'J';
	case color::none: return '_';
	}
	return '*';
}
void player::to_field_text(unsigned char* buffer) const
{
	for (auto row = 0; row < rows2; row++)
	{
		for (auto col = 0; col < cols; col++)
		{
			buffer[row * cols + col] = to_color_text(field_cells_[row][col].get_recognize_color());
		}
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
	SPDLOG_LOGGER_TRACE(logger, "game p:{} mse1:{} mse2:{} mse3:{} mse4:{}",
		player_idx_,
		nxt_cells_[0][axis].get_mse(),
		nxt_cells_[0][child].get_mse(),
		nxt_cells_[1][axis].get_mse(),
		nxt_cells_[1][child].get_mse()
	);
}


