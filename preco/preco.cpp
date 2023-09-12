#include <filesystem>
#include <thread>

#include "settings.h"
#include "logger.h"

#include "capture_thread.h"
#include "game_thread.h"

void run_threads()
{
    const auto logger = spdlog::get(kLoggerMain);

    // スレッド開始
    const auto game_thread_ptr = std::make_shared<game_thread>();
    std::thread th_game{ [&] { game_thread_ptr->run(); } };

	const auto capture_thread_ptr = std::make_shared<CaptureThread>(game_thread_ptr);
    std::thread th_capture{ [&] { capture_thread_ptr->run(); } };

    // キー押下待ち
    logger->info("Push any key to exit.");
    const auto c = getchar();
    logger->info(c);

    // スレッド終了要求
    capture_thread_ptr->request_end();
    game_thread_ptr->request_end();

    // スレッド終了待ち
    th_capture.join();
    th_game.join();
}

int main()
{
    // 設定とロガーの初期化
    const auto settings_instance = Settings::get_instance();
    settings_instance->init();
    init_logger();

	// 開始ログ
    const auto logger = spdlog::get(kLoggerMain);
    SPDLOG_LOGGER_DEBUG(logger, "********** application start **********");
    logger->info(settings_instance->dump());

    // スレッド実行
    run_threads();

    // 終了ログ
    SPDLOG_LOGGER_DEBUG(logger, "********** application end **********");
    spdlog::drop_all();
}



