#include <iostream>
#include <fstream>
#include <filesystem>

#include <nlohmann/json.hpp>
using json = nlohmann::json;

#include "logger.h"

// ========================================================
// settings
// ========================================================
nlohmann::json settings;
const char* SETTINGS_FILE_NAME = "settings.json";
const char* LOGS_PATH = "LOGS_PATH";

// ========================================================
// functions
// ========================================================
void initSettings() {

    /*
    if (std::filesystem::exists(SETTINGS_FILE_NAME)) {
        std::ifstream ifs(SETTINGS_FILE_NAME);
        settings = json::parse(ifs);
        return;
    }
    */
    
    settings[LOGS_PATH] = "logs/daily.txt";

    std::ofstream ofs(SETTINGS_FILE_NAME);
    ofs << std::setw(4) << settings << std::endl;
}

int main()
{
    initSettings();
    initLogger(settings[LOGS_PATH].get<std::string>());

    auto logger = spdlog::get(LOGGER_MAIN);
    logger->info("********** application start **********");

    logger->info(settings.dump(4));

    logger->info("********** application end **********");
    spdlog::drop_all();
}



