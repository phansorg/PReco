#include "settings.h"

#include <fstream>

#include "capture_thread.h"

constexpr auto settings_file_name = "settings.json";
constexpr auto json_indent = 4;

settings* settings::instance_ = nullptr;

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
    json["capture_start_no"] = 4330;
    json["capture_last_no"] = 4340;
    json["history_dir"] = "D:/puyo/movie/history";
    json["log_path"] = "D:/puyo/movie/logs/log.txt";

    std::ofstream ofs(settings_file_name);
    ofs << std::setw(json_indent) << json << std::endl;
}

std::string settings::dump() const
{
    return json.dump(json_indent);
}
