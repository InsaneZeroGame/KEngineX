#include "KLogger.h"

#include <stdio.h>
#include <io.h>
#include <fcntl.h>
#include <Windows.h>


KFramework::KLogger::KLogger()
{

    AllocConsole();

    HANDLE handle_out = GetStdHandle(STD_OUTPUT_HANDLE);
    int hCrt = _open_osfhandle(reinterpret_cast<uint64_t>(handle_out), _O_TEXT);
    FILE* hf_out = _fdopen(hCrt, "w");
    setvbuf(hf_out, NULL, _IONBF, 1);
    *stdout = *hf_out;

    HANDLE handle_in = GetStdHandle(STD_INPUT_HANDLE);
    hCrt = _open_osfhandle(reinterpret_cast<uint64_t>(handle_in), _O_TEXT);
    FILE* hf_in = _fdopen(hCrt, "r");
    setvbuf(hf_in, NULL, _IONBF, 128);
    *stdin = *hf_in;

    // create color multi threaded logger
    m_logger = spdlog::stdout_color_mt("Thanks to spdlog");
    //m_logger->info("Welcome to spdlog version {}.{}.{} !", SPDLOG_VER_MAJOR, SPDLOG_VER_MINOR, SPDLOG_VER_PATCH);
    //m_logger->error("Some error message with arg: {}", 1);

    //auto err_logger = spdlog::stderr_color_mt("stderr");
    //err_logger->error("Some error message");

}

KFramework::KLogger::~KLogger()
{

}
