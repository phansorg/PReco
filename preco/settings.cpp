#include "settings.h"

#include <fstream>

#include "capture_thread.h"

constexpr auto settings_file_name = "settings.json";
constexpr auto json_indent = 4;

settings* settings::instance_ = nullptr;
#ifdef _DEBUG
bool settings::debug = true;
#else
bool settings::debug = false;
#endif


void settings::init()
{
    /*
    // �ݒ�t�@�C�������݂���ꍇ�A�ǂݍ���
	if (std::filesystem::exists(settings_file_name)) {
	    std::ifstream ifs(settings_file_name);
        json = nlohmann::json::parse(ifs);
	    return;
	}
	*/

    // �ݒ�t�@�C�������݂��Ȃ��ꍇ�A�f�t�H���g�l�ō쐬
    json["capture_mode"] = capture_mode::jpeg;
    json["capture_path"] = "D:/puyo/movie/image_target";
    json["capture_start_no"] = 4330;
    json["capture_last_no"] = 4340;

	json["history_dir"] = "D:/puyo/movie/history";

    json["log_path"] = "D:/puyo/movie/logs/log.txt";

    json["recognize_debug_mode"] = true;
    json["recognize_debug_path"] = "D:/puyo/movie/image_debug";
    json["recognize_field_width"] = 258;
    json["recognize_field_height"] = 480;
    json["recognize_field_x"] = 186;
    json["recognize_field_y"] = 106;

    std::ofstream ofs(settings_file_name);
    ofs << std::setw(json_indent) << json << std::endl;
}

std::string settings::dump() const
{
    return json.dump(json_indent);
}
