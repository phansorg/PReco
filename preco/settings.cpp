#include "settings.h"

#include <fstream>

#include "capture_thread.h"
#include "game_thread.h"

constexpr auto settings_file_name = "settings.json";
constexpr auto json_indent = 4;

Settings* Settings::instance_ = nullptr;
#ifdef _DEBUG
bool settings::debug = true;
#else
bool Settings::debug_ = false;
#endif


void Settings::init()
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
	json_["capture_mode"] = CaptureMode::kJpeg;
    json_["capture_path"] = "D:/puyo/movie/image_target";
    json_["capture_start_no"] = 4210;
    //json["capture_start_no"] = 6000;
    json_["capture_last_no"] = 11085;

    json_["log_path"] = "D:/puyo/movie/logs/log.txt";

    json_["player_history_dir"] = "D:/puyo/movie/history";
    json_["player_field_x"] = { 186, 837 };
    json_["player_field_y"] = 106;
    json_["player_field_w"] = 258;
    json_["player_field_h"] = 480;
    json_["player_nxt1_x"] = { 479, 757 };
    json_["player_nxt1_y"] = 109;
    json_["player_nxt2_x"] = { 514, 732 };
    json_["player_nxt2_y"] = 196;
    json_["player_score_x"] = { 234, 833 };
    json_["player_score_y"] = 589;

    json_["game_debug_write"] = true;
    json_["game_debug_path"] = "D:/puyo/movie/image_debug";
    json_["game_start_mode"] = GameMode::kWaitCharacterSelection;
    //json["game_start_mode"] = game_mode::wait_game_reset;

    std::ofstream ofs(settings_file_name);
    ofs << std::setw(json_indent) << json_ << std::endl;
}

std::string Settings::dump() const
{
    return json_.dump(json_indent);
}
