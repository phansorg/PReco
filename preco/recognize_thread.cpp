#include "recognize_thread.h"
#include "base_thread.h"

#include "settings.h"
#include "logger.h"

recognize_thread::recognize_thread()
{
	thread_loop_ = true;

	auto& json = settings::get_instance()->json;
	cur_no_ = json["capture_start_no"].get<int>();
	debug_write_ = json["recognize_debug_write"].get<bool>();
	debug_path_ = json["recognize_debug_path"].get<std::string>();
	mode_ = static_cast<recognize_mode>(json["recognize_start_mode"].get<int>());
	field_width_ = json["recognize_field_width"].get<int>();
	field_height_ = json["recognize_field_height"].get<int>();
	field_y_ = json["recognize_field_y"].get<int>();

	capture_end_ = false;

	players_.push_back(std::make_unique<player>(player::p1));
	players_.push_back(std::make_unique<player>(player::p2));
}

void recognize_thread::run()
{
	const auto logger = spdlog::get(logger_main);
	SPDLOG_LOGGER_DEBUG(logger, "start");

	while (thread_loop_)
	{
		process();
		SPDLOG_LOGGER_TRACE(logger, "sleep");
		std::this_thread::sleep_for(std::chrono::milliseconds(thread_sleep_ms));
	}

	SPDLOG_LOGGER_DEBUG(logger, "end");
}

void recognize_thread::request_end()
{
	thread_loop_ = false;
}

void recognize_thread::set_capture_end()
{
	capture_end_ = true;
}

bool recognize_thread::is_mat_queue_max() const
{
	return mat_queue_.size() >= mat_queue_max_size_;
}

void recognize_thread::add_mat_queue(const cv::Mat& mat)
{
	std::lock_guard lock(mat_queue_mutex_);
	mat_queue_.push(mat);
}

void recognize_thread::process()
{
	while(!mat_queue_.empty())
	{
		cv::Mat org_mat = pop();

		switch(mode_)
		{
		case recognize_mode::wait_character_select:
			wait_character_select(org_mat);
			break;
		case recognize_mode::wait_reset:
			wait_reset(org_mat);
			break;
		case recognize_mode::wait_init:
			debug_init_game(org_mat);
			break;
		}

		cur_no_++;
	}

	if (capture_end_&& mat_queue_.empty())
	{
		request_end();
	}
}

cv::Mat recognize_thread::pop()
{
	std::lock_guard lock(mat_queue_mutex_);
	cv::Mat org_mat = mat_queue_.front();
	mat_queue_.pop();
	return org_mat;
}

void recognize_thread::wait_character_select(const cv::Mat& org_mat)
{
	const auto logger = spdlog::get(logger_main);

	std::vector<cv::Mat> channels;
	cv::Point* pos = nullptr;

	const auto width = field_width_;
	const auto height = field_height_ / 2;

	// 1P�Ֆʂ̏㔼���̈悪�S�ĐԂł����OK
	const auto& p1 = players_[player::p1];
	auto x = p1->field_x;
	auto y = field_y_;
	auto roi_mat = org_mat((cv::Rect(x, y, width, height)));
	split(roi_mat, channels);
	if (!checkRange(channels[b], true, pos, 0, 100)) return;
	if (!checkRange(channels[g], true, pos, 0, 100)) return;
	if (!checkRange(channels[r], true, pos, 180, 255)) return;

	// 2P�Ֆʂ̉������̈悪�S�ė΂ł����OK
	const auto& p2 = players_[player::p2];
	x = p2->field_x;
	y = field_y_ + field_height_ / 2;
	roi_mat = org_mat((cv::Rect(x, y, width, height)));
	split(roi_mat, channels);
	if (!checkRange(channels[b], true, pos, 0, 100)) return;
	if (!checkRange(channels[g], true, pos, 180, 255)) return;
	if (!checkRange(channels[r], true, pos, 0, 100)) return;

	// ���Z�b�g�҂��ɑJ��
	mode_ = recognize_mode::wait_reset;
	logger->info("No:{} recognize_mode:wait_reset", cur_no_);
}

void recognize_thread::wait_reset(const cv::Mat& org_mat)
{
	const auto logger = spdlog::get(logger_main);

	cv::Point* pos = nullptr;

	const auto y = field_y_ + field_height_ / 2;
	const auto width = field_width_ / 10;
	const auto height = field_height_ / 10;

	// �Ֆʂ̒����̈悪�S�č��ł����OK
	for (const auto& player : players_)
	{
		const auto x = player->field_x + field_width_ / 2;
		if (!checkRange(org_mat(cv::Rect(x, y, width, height)), true, pos, 0, 30)) return;
	}

	// �������҂��ɑJ��
	mode_ = recognize_mode::wait_init;
	logger->info("No:{} recognize_mode:wait_init", cur_no_);
}

void recognize_thread::debug_init_game(const cv::Mat& org_mat) const
{
	if (!settings::debug)
	{
		return;
	}

	auto debug_mat = org_mat.clone();

	// field�̐���`��
	for (const auto& player : players_)
	{
		const auto x1 = player->field_x;
		const auto y1 = field_y_;
		const auto x2 = player->field_x + field_width_;
		const auto y2 = field_y_ + field_height_;
		rectangle(debug_mat, cv::Point(x1, y1), cv::Point(x2, y2), cv::Scalar(0, 0, 255), 1);
	}

	// �o�̓t�@�C����
	std::ostringstream file_name;
	file_name << cur_no_ << ".png";

	// �f�B���N�g���p�X�Əo�̓t�@�C����������
	std::filesystem::path file_path = debug_path_;
	file_path.append(file_name.str());

	// �t�@�C���o��
	imwrite(file_path.string(), debug_mat);
}
