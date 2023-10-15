#include "cell.h"

#include <filesystem>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

#include "debug_writer.h"
#include "logger.h"
#include "settings.h"

Cell::Cell()
{
	type_ = CellType::kNone;
	row_ = -1;
	col_ = -1;

	frame_rect_ = cv::Rect(0, 0, 0, 0);
	recognize_rect_ = cv::Rect(0, 0, 0, 0);

	game_color_ = color::kNone;
	recognize_color_ = color::kNone;
	mse_ring_buffer_ = RingBuffer(Settings::mse_init_, Settings::history_max_);
	stabilize_count_ = 0;

	auto& json = Settings::get_instance()->json_;
	debug_path_ = json["cell_debug_path"].get<std::string>();
}

void Cell::set_rect(const cv::Rect in_rect)
{
	frame_rect_.x = in_rect.x;
	frame_rect_.y = in_rect.y;
	frame_rect_.width = in_rect.width;
	frame_rect_.height = in_rect.height;

	switch (type_)
	{
	case CellType::kNone:
		recognize_rect_.x = frame_rect_.x + frame_rect_.width / 4;
		recognize_rect_.y = frame_rect_.y + frame_rect_.height / 4;
		recognize_rect_.width = frame_rect_.width / 2;
		recognize_rect_.height = frame_rect_.height / 2;
		break;

	case CellType::kBlock:
		// ぷよ全般
		recognize_rect_.x = frame_rect_.x + frame_rect_.width * 15 / 100;
		recognize_rect_.y = frame_rect_.y + frame_rect_.height * 15 / 100;
		recognize_rect_.width = frame_rect_.width * 70 / 100;
		recognize_rect_.height = frame_rect_.height * 70 / 100;
		break;
	}

}

void Cell::reset()
{
	game_color_ = color::kNone;
	recognize_color_ = color::kNone;
	mse_ring_buffer_.reset(Settings::mse_init_);
	stabilize_count_ = 0;
}

void Cell::update_recognize_color(const cv::Scalar& bgr_scalar)
{
	if (row_ >= 12) return;
	if (row_ == 11 && col_ == 2) return;

	const auto r_val = static_cast<int>(bgr_scalar[r_]);
	const auto g_val = static_cast<int>(bgr_scalar[g_]);
	const auto b_val = static_cast<int>(bgr_scalar[b_]);

	auto recognize_color = color::kNone;
	if (r_val > 160 && g_val < 140 && b_val < 140)
		recognize_color = color::kR;
	else if (r_val < 140 && g_val > 173 && b_val < 120)
		recognize_color = color::kG;
	else if (r_val < 140 && g_val < 160 && b_val > 170)
		recognize_color = color::kB;
	else if (r_val > 189 && g_val > 171 && b_val < 150)
		recognize_color = color::kY;
	else if (r_val > 140 && g_val < 140 && b_val > 170)
		recognize_color = color::kP;
	else if (r_val > 168 && g_val > 173 && b_val > 173)
		recognize_color = color::kJam;
	recognize_color_ = recognize_color;

	const auto logger = spdlog::get(kLoggerMain);
	SPDLOG_LOGGER_TRACE(logger, "row:{} col:{} r:{} g:{} b:{} color:{}",
		row_,
		col_,
		static_cast<int>(r_val),
		static_cast<int>(g_val),
		static_cast<int>(b_val),
		static_cast<int>(recognize_color_)
	);
}

color Cell::get_recognize_color() const
{
	return recognize_color_;
}

void Cell::set_mse(const cv::Scalar& hsv_scalar)
{
	const auto mse = static_cast<int>(hsv_scalar[h_] + hsv_scalar[s_] + hsv_scalar[v_]);
	mse_ring_buffer_.next_record();
	mse_ring_buffer_.set(mse);
	
	if (mse >= mse_threshold_)
	{
		stabilize_count_ = 0;
		return;
	}

	stabilize_count_++;
}

int Cell::get_mse() const
{
	return mse_ring_buffer_.get();
}

bool Cell::is_stabilizing() const
{
	return stabilize_count_ >= 1;
}

bool Cell::is_stabilized() const
{
	return stabilize_count_ >= stabilized_threshold_count_;
}

void Cell::debug_render(const cv::Mat& debug_mat) const
{
	//if (recognize_color_ != color::kNone && type_ == CellType::kBlock)
	if (type_ == CellType::kBlock)
	{
		// 画像切り出し
		std::ostringstream file_name;
		auto class_id = static_cast<int>(recognize_color_);
		file_name << class_id << "/" << class_id << "_" << debug_writer::seq_++ << ".png";
		if (debug_writer::seq_ % 13 == 0)
		{
			// ディレクトリパスと出力ファイル名を結合
			std::filesystem::path file_path = debug_path_;
			file_path.append(file_name.str());

			// ファイル出力
			cv::Mat roi_image(debug_mat, cv::Rect(frame_rect_.x, frame_rect_.y, frame_rect_.width, frame_rect_.height));
			cv::imwrite(file_path.string(), roi_image);
		}
	}

	// frame_rect
	cv::Scalar bgr_scalar;
	switch (recognize_color_)
	{
	case color::kNone:
		bgr_scalar = cv::Scalar(0, 0, 0);
		break;
	case color::kR:
		bgr_scalar = cv::Scalar(0, 0, 255);
		break;
	case color::kG:
		bgr_scalar = cv::Scalar(0, 255, 0);
		break;
	case color::kB:
		bgr_scalar = cv::Scalar(255, 128, 0);
		break;
	case color::kY:
		bgr_scalar = cv::Scalar(0, 255, 255);
		break;
	case color::kP:
		bgr_scalar = cv::Scalar(255, 0, 192);
		break;
	case color::kJam:
		bgr_scalar = cv::Scalar(255, 255, 255);
		break;
	}
	render_rect(debug_mat, frame_rect_, bgr_scalar);

	// recognize_rect
	if (is_stabilized())
		bgr_scalar = cv::Scalar(0, 0, 0);
	else if (is_stabilizing())
		bgr_scalar = cv::Scalar(0, 255, 0);
	else
		bgr_scalar = cv::Scalar(0, 0, 255);
	render_rect(debug_mat, recognize_rect_, bgr_scalar);
}
void Cell::render_rect(const cv::Mat& debug_mat, const cv::Rect rect, const cv::Scalar& bgr_scalar) const
{
	const auto x1 = rect.x;
	const auto y1 = rect.y;
	const auto x2 = rect.x + rect.width - 1;
	const auto y2 = rect.y + rect.height - 1;
	rectangle(debug_mat, cv::Point(x1, y1), cv::Point(x2, y2), bgr_scalar, 2);
}
