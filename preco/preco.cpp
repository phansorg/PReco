#include <fstream>
#include <filesystem>
#include <thread>

#include <nlohmann/json.hpp>
using json = nlohmann::json;

#include "logger.h"

#include "capture_thread.h"
#include "recognize_thread.h"

// ========================================================
// settings
// ========================================================
nlohmann::json settings;
constexpr auto settings_file_name = "settings.json";
constexpr auto json_indent = 4;

constexpr auto capture_mode_key = "capture_mode";
constexpr auto capture_path_key = "capture_path";
constexpr auto capture_start_no_key = "capture_start_no";
constexpr auto capture_last_no_key = "capture_last_no";
constexpr auto history_dir_key = "history_dir";
constexpr auto log_path_key = "log_path";

// ========================================================
// functions
// ========================================================
void init_settings() {

    /*
    if (std::filesystem::exists(settings_file_name)) {
        std::ifstream ifs(settings_file_name);
        settings = json::parse(ifs);
        return;
    }
    */

    settings[capture_mode_key] = capture_mode::jpeg;
    settings[capture_path_key] = "D:/puyo/movie/image_target";
    settings[capture_start_no_key] = 4330;
    settings[capture_last_no_key] = 4340;
    settings[history_dir_key] = "D:/puyo/movie/history";
	settings[log_path_key] = "D:/puyo/movie/logs/log.txt";

    std::ofstream ofs(settings_file_name);
    ofs << std::setw(json_indent) << settings << std::endl;
}

void run_threads()
{
    const auto logger = spdlog::get(logger_main);

    // スレッド開始
    const auto recognize_thread_ptr = std::make_shared<recognize_thread>();
    std::thread th_recognize{ [&] { recognize_thread_ptr->run(); } };

	const auto capture_thread_ptr = std::make_shared<capture_thread>(
        recognize_thread_ptr,
        static_cast<capture_mode>(settings[capture_mode_key].get<int>()),
        settings[capture_path_key].get<std::string>(),
        settings[capture_start_no_key].get<int>(),
        settings[capture_last_no_key].get<int>()
    );
    std::thread th_capture{ [&] { capture_thread_ptr->run(); } };

    // キー押下待ち
    logger->info("Push any key to exit.");
    const auto c = getchar();
    logger->info(c);

    // スレッド終了要求
    capture_thread_ptr->request_end();
    recognize_thread_ptr->request_end();

    // スレッド終了待ち
    th_capture.join();
    th_recognize.join();
}

int main()
{
    // 設定とロガーの初期化
    init_settings();
    init_logger(settings[log_path_key].get<std::string>());

	// 開始ログ
    const auto logger = spdlog::get(logger_main);
    SPDLOG_LOGGER_DEBUG(logger, "********** application start **********");
    logger->info(settings.dump(json_indent));

    // スレッド実行
    run_threads();

    // 終了ログ
    SPDLOG_LOGGER_DEBUG(logger, "********** application end **********");
    spdlog::drop_all();
}



