#include "Logger.hpp"

#include <unistd.h>

#include <cstdio>
#include <ctime>
#include <filesystem>
#include <iostream>

namespace fenc {

static std::string g_logPath;

void Logger::init(const std::string& logPath) {
    g_logPath = logPath;
    std::filesystem::path p(logPath);
    if (p.has_parent_path()) {
        std::error_code ec;
        std::filesystem::create_directories(p.parent_path(), ec);
    }
}

void Logger::write(const char* level, const std::string& msg) {
    std::time_t t = std::time(nullptr);
    char ts[32];
    std::strftime(ts, sizeof(ts), "%Y-%m-%d %H:%M:%S", std::localtime(&t));

    char line[2048];
    int n = std::snprintf(line, sizeof(line), "[%s] [%s] [pid %d] %s\n", ts, level,
                          static_cast<int>(getpid()), msg.c_str());
    if (n < 0) return;
    std::size_t len = static_cast<std::size_t>(n < 2048 ? n : 2047);

    // One fwrite on an append-mode stream => one write per log line.
    if (!g_logPath.empty()) {
        if (FILE* f = std::fopen(g_logPath.c_str(), "a")) {
            std::fwrite(line, 1, len, f);
            std::fclose(f);
        }
    }
    std::string lv(level);
    std::ostream& os = (lv == "ERROR") ? std::cerr : std::cout;
    os.write(line, static_cast<std::streamsize>(len));
    os.flush();
}

void Logger::info(const std::string& msg) { write("INFO", msg); }
void Logger::error(const std::string& msg) { write("ERROR", msg); }

}  // namespace fenc
