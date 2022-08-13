#include <iostream>
#include <fstream>
#include <filesystem>

#include <nlohmann/json.hpp>
using json = nlohmann::json;

nlohmann::json settings;
const char* SETTINGS_FILE_NAME = "settings.json";
const char* LOGS_DIRECTORY = "logs_directory";

void initSettings() {

    if (std::filesystem::exists(SETTINGS_FILE_NAME)) {
        std::ifstream ifs(SETTINGS_FILE_NAME);
        settings = json::parse(ifs);
        return;
    }
    
    settings[LOGS_DIRECTORY] = "./logs";

    std::ofstream ofs(SETTINGS_FILE_NAME);
    ofs << std::setw(4) << settings << std::endl;
}

int main()
{
    initSettings();
    std::cout << settings[LOGS_DIRECTORY].dump(4) << std::endl;

    std::cout << "Hello World!" << std::endl;
}



