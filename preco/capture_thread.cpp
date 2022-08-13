#include "capture_thread.h"
#include "logger.h"

void capture_thread::run() const
{
    while (thread_loop)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

void capture_thread::request_stop()
{
    thread_loop = false;
}
