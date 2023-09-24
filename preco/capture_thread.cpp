#include "capture_thread.h"
#include "base_thread.h"

#include "settings.h"
#include "logger.h"

#include <filesystem>

CaptureThread::CaptureThread(const std::shared_ptr<game_thread>& game_thread_ptr) {

	thread_loop_ = true;

	game_thread_ptr_ = game_thread_ptr;

	auto& json = Settings::get_instance()->json_;
	mode_ = static_cast<CaptureMode>(json["capture_mode"].get<int>());
	path_ = json["capture_path"].get<std::string>();
	start_no_ = json["capture_start_no"].get<int>();
	last_no_ = json["capture_last_no"].get<int>();

	cur_no_ = start_no_;
}

void CaptureThread::run()
{
	const auto logger = spdlog::get(kLoggerMain);
	SPDLOG_LOGGER_DEBUG(logger, "start");

	while (thread_loop_)
    {
		process();
		SPDLOG_LOGGER_TRACE(logger, "sleep");
		std::this_thread::sleep_for(std::chrono::milliseconds(kThreadSleepMs));
    }

	// ���C���X���b�h�̃L�[�����ł͂Ȃ��A
	// ���X���b�h�Ń��[�v�I�������𖞂������ꍇ�̒ʒm
	game_thread_ptr_->set_capture_end();

	SPDLOG_LOGGER_DEBUG(logger, "end");
}

void CaptureThread::request_end()
{
    thread_loop_ = false;
}

void CaptureThread::process()
{
	switch(mode_)
	{
	case CaptureMode::kJpeg:
		read_jpeg();
		break;
	}
}

void CaptureThread::read_jpeg()
{
	const auto logger = spdlog::get(kLoggerMain);

	for (; cur_no_ <= last_no_; cur_no_++)
	{
		// �㑱�X���b�h�̃L���[�����܂��Ă���ꍇ�A��U������
		if (game_thread_ptr_->is_capture_mat_queue_max())
		{
			return;
		}

		// jpeg�t�@�C����
		std::ostringstream zero_padding;
		//zero_padding << std::setfill('0') << std::setw(jpeg_file_zero_count_) << cur_no_ << ".jpg";
		zero_padding << cur_no_ << ".jpg";

		// �f�B���N�g���p�X��jpeg�t�@�C����������
		std::filesystem::path file_path = path_;
		file_path.append(zero_padding.str());

		// jpeg�t�@�C���ǂݍ���
		auto mat = cv::imread(file_path.string());

		// �㑱�X���b�h�̃L���[�ɒǉ�
		game_thread_ptr_->add_capture_mat_queue(mat);
	}

	// �S�t�@�C�����������ꍇ�A�X���b�h���[�v�I��
	request_end();
}
