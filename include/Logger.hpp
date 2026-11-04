#pragma once
#include <string>

namespace fenc {

// Process-safe logger: every line is appended to the log file in a single
// write, so forked workers can log concurrently without interleaving.
class Logger {
public:
    static void init(const std::string& logPath);
    static void info(const std::string& msg);
    static void error(const std::string& msg);

private:
    static void write(const char* level, const std::string& msg);
};

}  // namespace fenc
