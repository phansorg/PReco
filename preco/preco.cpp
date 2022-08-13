#include <fstream>
#include <filesystem>
#include <thread>

#include <nlohmann/json.hpp>
using json = nlohmann::json;

#include "logger.h"

#include "capture_thread.h"

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
    settings[capture_last_no_key] = 4330;
    settings[history_dir_key] = "D:/puyo/movie/history";
	settings[log_path_key] = "D:/puyo/movie/logs/log.txt";

    std::ofstream ofs(settings_file_name);
    ofs << std::setw(json_indent) << settings << std::endl;
}

void start_thread()
{
    const auto logger = spdlog::get(logger_main);

	auto capture = capture_thread(
        static_cast<capture_mode>(settings[capture_mode_key].get<int>()),
        settings[capture_path_key].get<std::string>(),
        settings[capture_start_no_key].get<int>(),
        settings[capture_last_no_key].get<int>()
    );

    std::thread th_capture{ [&] { capture.run(); } };

    logger->info("Push any key to exit.");
    const auto c = getchar();
    logger->info(c);

    capture.request_stop();

    th_capture.join();
}

int main()
{
    init_settings();
    init_logger(settings[log_path_key].get<std::string>());
    const auto logger = spdlog::get(logger_main);
    logger->debug("********** application start **********");
    logger->info(settings.dump(json_indent));

    start_thread();

    logger->debug("********** application end **********");
    spdlog::drop_all();
}



