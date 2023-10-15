#pragma once

#ifdef _DEBUG
//#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_TRACE
#endif

#include <spdlog/spdlog.h>

constexpr auto kLoggerMain = "main";

void init_logger();
