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
    // 設定ファイルが存在する場合、読み込む
	if (std::filesystem::exists(settings_file_name)) {
	    std::ifstream ifs(settings_file_name);
        json = nlohmann::json::parse(ifs);
	    return;
	}
	*/

    // 設定ファイルが存在しない場合、デフォルト値で作成
    json["capture_mode"] = capture_mode::jpeg;
    json["capture_path"] = "D:/puyo/movie/image_target";
    json["capture_start_no"] = 4210;
    json["capture_last_no"] = 4400;

	json["history_dir"] = "D:/puyo/movie/history";

    json["log_path"] = "D:/puyo/movie/logs/log.txt";

    json["player_field_x"] = { 186, 837 };
    json["player_field_y"] = 106;
    json["player_field_w"] = 258;
    json["player_field_h"] = 480;
    json["player_nex1_x"] = { 479, 757 };
    json["player_nex1_y"] = 109;
    json["player_nex2_x"] = { 514, 732 };
    json["player_nex2_y"] = 196;

    json["game_debug_write"] = true;
    json["game_debug_path"] = "D:/puyo/movie/image_debug";
    json["game_start_mode"] = game_mode::wait_character_selection;
    json["game_histories_size"] = 5;

    std::ofstream ofs(settings_file_name);
    ofs << std::setw(json_indent) << json << std::endl;
}

std::string settings::dump() const
{
    return json.dump(json_indent);
}
