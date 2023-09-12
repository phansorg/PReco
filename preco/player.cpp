#include "player.h"

#include <opencv2/imgproc.hpp>
#include <iostream>
#include <fstream>

#include "settings.h"
#include "logger.h"

Player::Player(const int player_idx)
{
	player_idx_ = player_idx;
	player_mode_ = PlayerMode::kWaitGameStart;
	cur_records_idx_ = -2;

	auto& json = Settings::get_instance()->json_;
	history_dir_ = json["player_history_dir"].get<std::string>();

	// ディレクトリパスと出力ファイル名を結合
	std::ostringstream file_name;
	file_name << "game_record_" << player_idx_ << ".txt";
	game_record_path_ = history_dir_;
	game_record_path_.append(file_name.str());

	// field
	field_frame_rect_ = cv::Rect(
		json["player_field_x"][player_idx_].get<int>(),
		json["player_field_y"].get<int>(),
		json["player_field_w"].get<int>(),
		json["player_field_h"].get<int>()
	);
	cell_width_ = field_frame_rect_.width / cols_;
	cell_height_ = field_frame_rect_.height / rows0_;

	// nxt1
	auto row = 0;
	auto col = axis_;
	nxt_cells_[row][col].type_ = CellType::kBlock;
	nxt_cells_[row][col].row_ = row;
	nxt_cells_[row][col].col_ = col;
	nxt_cells_[row][col].set_rect(cv::Rect(
		json["player_nxt1_x"][player_idx_].get<int>(),
		json["player_nxt1_y"].get<int>(),
		cell_width_,
		cell_height_
	));

	auto clone_rect = nxt_cells_[row][col].frame_rect_;
	col = child_;
	clone_rect.y += clone_rect.height;
	nxt_cells_[row][col].type_ = CellType::kBlock;
	nxt_cells_[row][col].row_ = row;
	nxt_cells_[row][col].col_ = col;
	nxt_cells_[row][col].set_rect(clone_rect);

	// nxt2
	row = 1;
	col = axis_;
	nxt_cells_[row][col].type_ = CellType::kBlock;
	nxt_cells_[row][col].row_ = row;
	nxt_cells_[row][col].col_ = col;
	nxt_cells_[row][col].set_rect(cv::Rect(
		json["player_nxt2_x"][player_idx_].get<int>(),
		json["player_nxt2_y"].get<int>(),
		cell_width_ * 4 / 5,
		cell_height_ * 4 / 5
	));
	clone_rect = nxt_cells_[row][col].frame_rect_;
	col = child_;
	clone_rect.y += clone_rect.height;
	nxt_cells_[row][col].type_ = CellType::kBlock;
	nxt_cells_[row][col].row_ = 1;
	nxt_cells_[row][col].col_ = col;
	nxt_cells_[row][col].set_rect(clone_rect);

	// combo
	combo_cell_.set_rect(cv::Rect(
		json["player_score_x"][player_idx_].get<int>() + 4,
		json["player_score_y"].get<int>() + 1,
		cell_width_ / 2,
		cell_height_ / 4
	));

	// その他Rectの計算
	init_field_cells();
	init_combo_cell();
	init_end_cell();
	init_wait_character_selection_rect();
	init_wait_reset_rect();

	// 1試合目から開始
	game_no_ = 1;
}

// ============================================================
// rect
// ============================================================
void Player::init_field_cells()
{
	const auto width = field_frame_rect_.width;
	const auto height = field_frame_rect_.height;
	for (int row = 0; row < rows2_; row++)
	{
		const auto y_idx = rows2_ - row - 3;
		const auto y1 = height * y_idx / rows0_ + field_frame_rect_.y;
		const auto y2 = height * (y_idx + 1) / rows0_ + field_frame_rect_.y;
		for (int col = 0; col < cols_; col++)
		{
			const auto x1 = width * col / cols_ + field_frame_rect_.x;
			const auto x2 = width * (col + 1) / cols_ + field_frame_rect_.x;
			field_cells_[row][col].type_ = CellType::kBlock;
			field_cells_[row][col].row_ = row;
			field_cells_[row][col].col_ = col;
			field_cells_[row][col].set_rect(cv::Rect(
				x1,
				y1,
				x2 - x1,
				y2 - y1
			));
		}
	}
}

void Player::init_combo_cell()
{
	// スコアの左端が変化したらcombo
	const auto width = cell_width_ / 2;
	const auto x = player_idx_ == p1_ ?
		field_frame_rect_.x : field_frame_rect_.x + field_frame_rect_.width - width;
	end_cell_.set_rect(cv::Rect(
		x,
		field_frame_rect_.y + field_frame_rect_.height + width / 2,
		width,
		width
	));
}

void Player::init_end_cell()
{
	// 角の領域が全て緑であればend
	const auto width = cell_width_ / 2;
	const auto x = player_idx_ == p1_ ?
		field_frame_rect_.x : field_frame_rect_.x + field_frame_rect_.width - width;
	end_cell_.set_rect(cv::Rect(
		x,
		field_frame_rect_.y + field_frame_rect_.height + width / 2,
		width,
		width
	));
}

void Player::init_wait_character_selection_rect()
{
	// 1P盤面の上半分領域が全て赤であればOK
	// 2P盤面の下半分領域が全て緑であればOK
	const auto y = player_idx_ == p1_ ? 
		field_frame_rect_.y : field_frame_rect_.y + field_frame_rect_.height / 2;
	wait_character_selection_rect_ = cv::Rect(
		field_frame_rect_.x,
		y,
		field_frame_rect_.width,
		field_frame_rect_.height / 2
	);
}

void Player::init_wait_reset_rect()
{
	// 盤面の中央領域が全て黒であればOK
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

bool Player::wait_character_selection(const cv::Mat& org_mat) const
{
	std::vector<cv::Mat> channels;
	cv::Point* pos = nullptr;

	const auto roi_mat = org_mat(wait_reset_rect_);
	split(roi_mat, channels);
	if (player_idx_ == p1_)
	{
		// 1P盤面の上半分領域が全て赤であればOK
		if (!checkRange(channels[Cell::b_], true, pos, 0, 100)) return false;
		if (!checkRange(channels[Cell::g_], true, pos, 0, 100)) return false;
		if (!checkRange(channels[Cell::r_], true, pos, 180, 255)) return false;
	}
	else
	{
		// 2P盤面の下半分領域が全て緑であればOK
		if (!checkRange(channels[Cell::b_], true, pos, 0, 100)) return false;
		if (!checkRange(channels[Cell::g_], true, pos, 180, 255)) return false;
		if (!checkRange(channels[Cell::r_], true, pos, 0, 100)) return false;
	}

	return true;
}

bool Player::wait_game_reset(const cv::Mat& org_mat)
{
	// 盤面の中央領域が全て黒であればOK
	cv::Point* pos = nullptr;
	if (const auto roi_mat = org_mat(wait_reset_rect_);
		!checkRange(roi_mat, true, pos, 0, 30)) return false;

	// fieldをリセット
	for (auto& field_row : field_cells_)
	{
		for (auto& field_cell : field_row)
		{
			field_cell.reset();
		}
	}

	// nxtをリセット
	for (auto& nxt_child_cells : nxt_cells_)
	{
		for (auto& nxt_cell : nxt_child_cells)
		{
			nxt_cell.reset();
		}
	}

	// 最初は操作ぷよが無い状態のため、-1でリセットする
	cur_records_idx_ = -1;
	operation_records_.clear();

	// comboをリセット
	combo_cell_.reset();

	// endをリセット
	end_cell_.reset();

	color_map_.reset();

	// 試合数を加算
	game_no_++;

	return true;
}

bool Player::wait_game_init(const cv::Mat& org_mat, const std::list<cv::Mat>& mat_histories)
{
	const auto logger = spdlog::get(kLoggerMain);

	// 初期化済みの場合、OK
	if (!operation_records_.empty())
	{
		SPDLOG_LOGGER_TRACE(logger, "wait_game_init ok p:{}", player_idx_);
		return true;
	}

	// nxtが全て安定し、色があればOK
	update_nxt_cells(org_mat, mat_histories);
	for (auto& nxt_child_cells : nxt_cells_)
	{
		for (auto& nxt_cell : nxt_child_cells)
		{
			if (!nxt_cell.is_stabilized()) return false;
			if (nxt_cell.get_recognize_color() == color::kNone) return false;
		}
	}

	// 初期化完了時のnxtを登録し、再度リセット(game_startで再度認識させるため)
	for (auto& nxt_child_cells : nxt_cells_)
	{
		const Operation operation_record{
			{
				nxt_child_cells[axis_].get_recognize_color(),
				nxt_child_cells[child_].get_recognize_color()
			},
			-1,
			-1
		};
		operation_records_.push_back(operation_record);
		color_map_.insert(operation_record.colors[axis_]);
		color_map_.insert(operation_record.colors[child_]);

		nxt_child_cells[axis_].reset();
		nxt_child_cells[child_].reset();
	}

	// historyのファイル名を設定
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

	// ディレクトリパスと出力ファイル名を結合
	history_path_ = history_dir_;
	history_path_.append(file_name.str());

	// player_modeをゲーム開始後にセット
	player_mode_ = PlayerMode::kWaitNxtStabilize;
	logger->info("p:{} player_mode:{}", player_idx_, static_cast<int>(player_mode_));

	return true;
}

bool Player::game(int cur_no, const cv::Mat& org_mat, const std::list<cv::Mat>& mat_histories)
{
	update_all_cells(org_mat, mat_histories);

	switch (player_mode_)
	{
	case PlayerMode::kWaitGameStart:
		break;
	case PlayerMode::kWaitNxtStabilize:
		wait_nxt_stabilize();
		break;
	case PlayerMode::kWaitNxtChange:
		if (wait_combo())
			break;
		wait_nxt_change();
		break;
	case PlayerMode::kWaitGameEnd:
		break;
	}

	return wait_game_end();
}

void Player::game_end()
{
	const auto logger = spdlog::get(kLoggerMain);
	append_game_record();
	player_mode_ = PlayerMode::kWaitGameStart;
	logger->info("p:{} player_mode:{}", player_idx_, static_cast<int>(player_mode_));
}

void Player::wait_nxt_stabilize()
{
	for (auto& nxt_child_cells : nxt_cells_)
	{
		for (auto& nxt_cell : nxt_child_cells)
		{
			if (!nxt_cell.is_stabilized()) return;
		}
	}

	const Operation operation_record{
		{
			nxt_cells_[1][axis_].get_recognize_color(),
			nxt_cells_[1][child_].get_recognize_color()
		},
		-1,
		-1
	};
	operation_records_.push_back(operation_record);
	color_map_.insert(operation_record.colors[axis_]);
	color_map_.insert(operation_record.colors[child_]);
	cur_records_idx_ += 1;

	const auto logger = spdlog::get(kLoggerMain);
	player_mode_ = PlayerMode::kWaitNxtChange;
	logger->info("p:{} player_mode:{}", player_idx_, static_cast<int>(player_mode_));
}

bool Player::wait_combo()
{
	// スコアの白色(jam)部分が無くなったら連鎖開始
	if (combo_cell_.get_recognize_color() == color::kJam) return false;

	// 消した後は、現時点では対応しない
	const auto logger = spdlog::get(kLoggerMain);
	player_mode_ = PlayerMode::kWaitGameEnd;
	logger->info("p:{} player_mode:{}", player_idx_, static_cast<int>(player_mode_));
	return true;
}

void Player::wait_nxt_change()
{
	for (auto& nxt_child_cells : nxt_cells_)
	{
		for (auto& nxt_cell : nxt_child_cells)
		{
			if (nxt_cell.is_stabilized()) return;
		}
	}

	put_nxt();
	write_history();

	const auto logger = spdlog::get(kLoggerMain);
	player_mode_ = PlayerMode::kWaitNxtStabilize;
	logger->info("p:{} player_mode:{}", player_idx_, static_cast<int>(player_mode_));
}

void Player::put_nxt()
{
	const auto logger = spdlog::get(kLoggerMain);

	auto& [colors, col, rotate] = operation_records_[cur_records_idx_];

	// nxtが変わった = 設置したのでfieldを再確認
	std::list<int> put_rows;
	std::list<int> put_cols;
	for (auto& field_row : field_cells_)
	{
		for (auto& field_cell : field_row)
		{
			const auto game_color = field_cell.game_color_;
			const auto recognize_color = field_cell.get_recognize_color();
			// 色が変化していない場合、対象外
			if (game_color == recognize_color) continue;

			// gameの状態として設定
			field_cell.game_color_ = recognize_color;

			// 軸が先頭になるよう追加
			if (colors[axis_] == recognize_color)
			{
				put_rows.push_front(field_cell.row_);
				put_cols.push_front(field_cell.col_);
			}
			else
			{
				put_rows.push_back(field_cell.row_);
				put_cols.push_back(field_cell.col_);
			}

			logger->info("put_nxt p:{} row:{} col:{} color:{}>{}",
				player_idx_,
				field_cell.row_,
				field_cell.col_,
				static_cast<int>(game_color),
				static_cast<int>(recognize_color));
		}
	}

	// ズレが発生した(変化が2箇所でない)場合、現時点では対応しない
	if (put_rows.size() != nxt_child_max_)
	{
		player_mode_ = PlayerMode::kWaitGameEnd;
		logger->info("p:{} player_mode:{}", player_idx_, static_cast<int>(player_mode_));
		return;
	}

	const auto axis_row = put_rows.front();
	const auto axis_col = put_cols.front();
	const auto child_row = put_rows.back();
	const auto child_col = put_cols.back();
	col = axis_col;
	if (child_col == axis_col + 1)
	{
		rotate = 1;
	}
	else if (child_col == axis_col - 1)
	{
		rotate = 3;
	}
	else if (child_col == axis_col)
	{
		if (child_row > axis_row)
		{
			rotate = 0;
		}
		else
		{
			rotate = 2;
		}
	}
	else
	{
		// ズレが発生した(列が離れている)場合、現時点では対応しない
		player_mode_ = PlayerMode::kWaitGameEnd;
		logger->info("p:{} player_mode:{}", player_idx_, static_cast<int>(player_mode_));
	}
}

bool Player::wait_game_end() const
{
	return end_cell_.get_recognize_color() == color::kG;
}

void Player::update_all_cells(const cv::Mat& org_mat, const std::list<cv::Mat>& mat_histories)
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

void Player::update_nxt_cells(const cv::Mat& org_mat, const std::list<cv::Mat>& mat_histories)
{
	for (auto& nxt_child_cells : nxt_cells_)
	{
		for (auto& nxt_cell : nxt_child_cells)
		{
			update_cell(org_mat, mat_histories, nxt_cell);
		}
	}
}

void Player::update_cell(const cv::Mat& org_mat, const std::list<cv::Mat>& mat_histories, Cell& target_cell) const
{
	// 対象領域を取得し、HSVに変換
	const auto& org_roi = org_mat(target_cell.recognize_rect_);
	cv::Mat org_hsv_mat;
	cvtColor(org_roi, org_hsv_mat, cv::COLOR_BGR2HSV);

	const auto history_roi = mat_histories.front()(target_cell.recognize_rect_);
	cv::Mat history_hsv_mat;
	cvtColor(history_roi, history_hsv_mat, cv::COLOR_BGR2HSV);

	// 対象領域のピクセル毎にMSEを計算
	cv::Mat diff_mat;
	subtract(org_hsv_mat, history_hsv_mat, diff_mat);
	cv::Mat pow_mat;
	pow(diff_mat, 2, pow_mat);

	// MSEの平均をとり、cellに設定
	const auto before_stabilized = target_cell.is_stabilized();
	target_cell.set_mse(mean(pow_mat));

	// 安定状態に遷移した場合、色を更新
	if (!before_stabilized && target_cell.is_stabilized())
	{
		target_cell.update_recognize_color(mean(org_roi));
	}
}

void Player::write_history() const
{
	std::ofstream history_file(history_path_, std::ios::app);
	if (!history_file)
	{
		return;
	}

	unsigned char field_text[rows2_ * cols_ + 1] = {0};
	to_field_text(field_text);

	history_file
		<< to_color_text(operation_records_[cur_records_idx_].colors[axis_])
		<< to_color_text(operation_records_[cur_records_idx_].colors[child_])
		<< ","
		<< to_color_text(operation_records_[cur_records_idx_ + 1].colors[axis_])
		<< to_color_text(operation_records_[cur_records_idx_ + 1].colors[child_])
		<< ","
		<< to_color_text(operation_records_[cur_records_idx_ + 2].colors[axis_])
		<< to_color_text(operation_records_[cur_records_idx_ + 2].colors[child_])
		<< ","
		<< operation_records_[cur_records_idx_].col
		<< operation_records_[cur_records_idx_].rotate
		<< ","
		<< field_text
		<< std::endl;
	history_file.close();
}

unsigned char Player::to_color_text(const color from_color)
{
	switch (from_color)
	{
	case color::kR: return 'R';
	case color::kG: return 'G';
	case color::kB: return 'B';
	case color::kY: return 'Y';
	case color::kP: return 'P';
	case color::kJam: return 'J';
	case color::kNone: return '_';
	}
	return '*';
}
void Player::to_field_text(unsigned char* buffer) const
{
	for (auto row = 0; row < rows2_; row++)
	{
		for (auto col = 0; col < cols_; col++)
		{
			buffer[row * cols_ + col] = to_color_text(field_cells_[row][col].get_recognize_color());
		}
	}
}

void Player::append_game_record() const
{
	std::ofstream game_record_file(game_record_path_, std::ios::app);
	if (!game_record_file)
	{
		return;
	}

	constexpr int require_nxt_count = 16;

	// 必要手数に満たないならスキップ
	if (cur_records_idx_ < require_nxt_count - 1) return;

	unsigned char nxt_key_array[require_nxt_count * 2 + 1] = { 0 };
	unsigned char nxt_actual_array[require_nxt_count * 2 + 1] = { 0 };
	unsigned char col_array[require_nxt_count + 1] = { 0 };
	unsigned char rotate_array[require_nxt_count + 1] = { 0 };
	for (auto idx = 0; idx < require_nxt_count; idx++)
	{
		const auto axis_color = operation_records_[idx].colors[axis_];
		const auto child_color = operation_records_[idx].colors[child_];
		const auto col = operation_records_[idx].col;
		const auto rotate = operation_records_[idx].rotate;
		if (axis_color == color::kNone ||
			child_color == color::kNone ||
			col < 0 ||
			rotate < 0) {
			game_record_file.close();
			return;
		}
		
		auto axis_key = color_map_.to_key(axis_color);
		auto child_key = color_map_.to_key(child_color);
		nxt_key_array[idx * 2] = std::min(axis_key, child_key);
		nxt_key_array[idx * 2 + 1] = std::max(axis_key, child_key);
		nxt_actual_array[idx * 2] = to_color_text(axis_color);
		nxt_actual_array[idx * 2 + 1] = to_color_text(child_color);
		col_array[idx] = '0' + col;
		rotate_array[idx] = '0' + rotate;
	}

	game_record_file
		<< 0
		<< ","
		<< 0
		<< ","
		<< game_no_
		<< ","
		<< nxt_key_array
		<< ","
		<< nxt_actual_array
		<< ","
		<< col_array
		<< ","
		<< rotate_array
		<< std::endl;

	game_record_file.close();
}

// ============================================================
// debug
// ============================================================

void Player::debug_render(const cv::Mat& debug_mat) const
{
	const auto logger = spdlog::get(kLoggerMain);

	// fieldの線を描画
	for (const auto& field_row : field_cells_)
	{
		for (const auto& field_cell : field_row)
		{
			field_cell.debug_render(debug_mat);
		}
	}
	// nxtの線を描画
	for (const auto& nxt_child_cells : nxt_cells_)
	{
		for (const auto& nxt_cell : nxt_child_cells)
		{
			nxt_cell.debug_render(debug_mat);
		}
	}

	// comboの線を描画
	combo_cell_.debug_render(debug_mat);

	// endの線を描画
	end_cell_.debug_render(debug_mat);

	// nxtのmse
	SPDLOG_LOGGER_TRACE(logger, "game p:{} mse1:{} mse2:{} mse3:{} mse4:{}",
		player_idx_,
		nxt_cells_[0][axis].get_mse(),
		nxt_cells_[0][child].get_mse(),
		nxt_cells_[1][axis].get_mse(),
		nxt_cells_[1][child].get_mse()
	);
}


