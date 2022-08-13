#pragma once
#include <spdlog/spdlog.h>

constexpr auto logger_main = "main";

void init_logger(const std::string& file_path);
