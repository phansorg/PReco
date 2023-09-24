#include "settings.h"

#include <fstream>

#include "capture_thread.h"
#include "game_thread.h"

constexpr auto settings_file_name = "settings.json";
constexpr auto json_indent = 4;

Settings* Settings::instance_ = nullptr;
#ifdef _DEBUG
bool Settings::debug_ = true;
#else
bool Settings::debug_ = false;
#endif


void Settings::init()
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
	json_["capture_mode"] = CaptureMode::kJpeg;
    json_["capture_path"] = "D:/puyo/movie/image_target";
    json_["capture_start_no"] = 1;
    //json_["capture_start_no"] = 3572;
    //json_["capture_start_no"] = 5924;
    json_["capture_last_no"] = 105435;

    json_["log_path"] = "D:/puyo/movie/logs/log.txt";

    json_["player_history_dir"] = "D:/puyo/movie/history";

	// ぷよの左端
    json_["player_field_x"] = { 232, 894 };
    // フィールドの上端
    json_["player_field_y"] = 309;
    // ぷよの右端まで
    json_["player_field_w"] = 264;
    // フィールドの下端まで
    json_["player_field_h"] = 486;

    // ネクストの左端
	json_["player_nxt1_x"] = { 532, 815 };
    // ネクストの上端
    json_["player_nxt1_y"] = 314;

    // ネクネクの左端
    json_["player_nxt2_x"] = { 567, 786 };
    // ネクネクの上端
    json_["player_nxt2_y"] = 404;

    // スコア最上位0の左端
	json_["player_score_x"] = { 282, 891 };
    // スコア最上位0の上端
	json_["player_score_y"] = 799;

    json_["game_debug_write"] = true;
    json_["game_debug_path"] = "D:/puyo/movie/image_debug";
    json_["game_start_mode"] = GameMode::kWaitCharacterSelection;
    //json_["game_start_mode"] = GameMode::kWaitGameReset;

    std::ofstream ofs(settings_file_name);
    ofs << std::setw(json_indent) << json_ << std::endl;
}

std::string Settings::dump() const
{
    return json_.dump(json_indent);
}
