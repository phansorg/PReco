#include <fstream>
#include <filesystem>

#include <nlohmann/json.hpp>
using json = nlohmann::json;

#include "logger.h"

// ========================================================
// settings
// ========================================================
nlohmann::json settings;
const char* settings_file_name = "settings.json";

const char* log_path = "log_path";

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

    settings[log_path] = "D:/puyo/movie/logs/log.txt";

    std::ofstream ofs(settings_file_name);
    ofs << std::setw(4) << settings << std::endl;
}

int main()
{
    init_settings();
    init_logger(settings[log_path].get<std::string>());

    const auto logger = spdlog::get(logger_main);
    logger->info("********** application start **********");
    logger->info(settings.dump(4));



    logger->info("********** application end **********");
    spdlog::drop_all();
}



