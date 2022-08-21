#include "settings.h"

#include <fstream>

#include "capture_thread.h"
#include "game_thread.h"

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
    json["capture_start_no"] = 4210;
    json["capture_last_no"] = 11085;

	json["history_dir"] = "D:/puyo/movie/history";

    json["log_path"] = "D:/puyo/movie/logs/log.txt";

    json["player_field_x"] = { 186, 837 };
    json["player_field_y"] = 106;
    json["player_field_w"] = 258;
    json["player_field_h"] = 480;
    json["player_nxt1_x"] = { 479, 757 };
    json["player_nxt1_y"] = 109;
    json["player_nxt2_x"] = { 514, 732 };
    json["player_nxt2_y"] = 196;
    json["player_score_x"] = { 234, 833 };
    json["player_score_y"] = 589;

    json["game_debug_write"] = true;
    json["game_debug_path"] = "D:/puyo/movie/image_debug";
    json["game_start_mode"] = game_mode::wait_character_selection;

    std::ofstream ofs(settings_file_name);
    ofs << std::setw(json_indent) << json << std::endl;
}

std::string settings::dump() const
{
    return json.dump(json_indent);
}
