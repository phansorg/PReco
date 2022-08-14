#include "capture_thread.h"
#include "base_thread.h"
#include "logger.h"

#include <filesystem>

capture_thread::capture_thread(
	const std::shared_ptr<recognize_thread>& recognize_thread_ptr,
	const capture_mode mode,
	const std::string& path,
	const int start_no,
	const int last_no) {

	thread_loop_ = true;

	recognize_thread_ptr_ = recognize_thread_ptr;
	mode_ = mode;
	path_ = path;
	start_no_ = start_no;
	last_no_ = last_no;

	cur_no_ = start_no;
}


void capture_thread::run()
{
	const auto logger = spdlog::get(logger_main);
	SPDLOG_LOGGER_DEBUG(logger, "start");

	while (thread_loop_)
    {
		process();
		SPDLOG_LOGGER_TRACE(logger, "sleep");
		std::this_thread::sleep_for(std::chrono::milliseconds(thread_sleep_ms));
    }

	// ���C���X���b�h�̃L�[�����ł͂Ȃ��A
	// ���X���b�h�Ń��[�v�I�������𖞂������ꍇ�̒ʒm
	recognize_thread_ptr_->set_capture_end();

	SPDLOG_LOGGER_DEBUG(logger, "end");
}

void capture_thread::request_end()
{
    thread_loop_ = false;
}

void capture_thread::process()
{
	switch(mode_)
	{
	case capture_mode::jpeg:
		read_jpeg();
		break;
	}
}

void capture_thread::read_jpeg()
{
	const auto logger = spdlog::get(logger_main);

	for (; cur_no_ <= last_no_; cur_no_++)
	{
		// �F���X���b�h�̃L���[�����܂��Ă���ꍇ�A��U������
		if (recognize_thread_ptr_->is_mat_queue_max())
		{
			return;
		}

		// jpeg�t�@�C����
		std::ostringstream zero_padding;
		zero_padding << std::setfill('0') << std::setw(jpeg_file_zero_count_) << cur_no_ << ".jpg";

		// �f�B���N�g���p�X��jpeg�t�@�C����������
		std::filesystem::path file_path = path_;
		file_path.append(zero_padding.str());

		// jpeg�t�@�C���ǂݍ���
		auto mat = cv::imread(file_path.string());
	}

	// �S�t�@�C�����������ꍇ�A�X���b�h���[�v�I��
	request_end();
}
