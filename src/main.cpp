#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <string>
#include <thread>

#include "Cipher.hpp"
#include "DirectoryScanner.hpp"
#include "EnvLoader.hpp"
#include "FileIO.hpp"
#include "Logger.hpp"
#include "ProcessManagement.hpp"

namespace fs = std::filesystem;
using namespace fenc;

static void usage(const char* prog) {
    std::cout <<
        "Usage: " << prog << " <encrypt|decrypt> <file-or-directory> [options]\n\n"
        "  <file>        single-file operation (runs in this process)\n"
        "  <directory>   recursive batch operation (parallel worker processes)\n\n"
        "Options:\n"
        "  -j, --jobs N   max parallel worker processes (default: CPU cores)\n"
        "  -e, --env PATH path to .env file (default: .env)\n"
        "  -l, --log PATH log file (default: logs/fileenc.log)\n"
        "  -h, --help     show this help\n";
}

int main(int argc, char** argv) {
    if (argc == 2 && (std::string(argv[1]) == "-h" || std::string(argv[1]) == "--help")) {
        usage(argv[0]);
        return 0;
    }
    if (argc < 3) {
        usage(argv[0]);
        return 1;
    }

    std::string modeStr = argv[1];
    fs::path target = argv[2];
    std::string envPath = ".env", logPath = "logs/fileenc.log";
    std::size_t jobs = std::thread::hardware_concurrency();
    if (jobs == 0) jobs = 2;

    for (int i = 3; i < argc; ++i) {
        std::string a = argv[i];
        auto next = [&](std::string& dst) {
            if (i + 1 >= argc) {
                std::cerr << "missing value for " << a << "\n";
                std::exit(1);
            }
            dst = argv[++i];
        };
        if (a == "-j" || a == "--jobs") {
            std::string v;
            next(v);
            int n = std::atoi(v.c_str());
            if (n < 1) {
                std::cerr << "--jobs must be >= 1\n";
                return 1;
            }
            jobs = static_cast<std::size_t>(n);
        } else if (a == "-e" || a == "--env") {
            next(envPath);
        } else if (a == "-l" || a == "--log") {
            next(logPath);
        } else if (a == "-h" || a == "--help") {
            usage(argv[0]);
            return 0;
        } else {
            std::cerr << "unknown option: " << a << "\n";
            usage(argv[0]);
            return 1;
        }
    }

    Mode mode;
    if (modeStr == "encrypt") mode = Mode::Encrypt;
    else if (modeStr == "decrypt") mode = Mode::Decrypt;
    else {
        usage(argv[0]);
        return 1;
    }

    Logger::init(logPath);

    int key;
    try {
        key = EnvLoader::getIntKey(EnvLoader::load(envPath), "ENCRYPTION_KEY");
    } catch (const std::exception& e) {
        Logger::error(std::string("key error: ") + e.what());
        return 1;
    }
    CaesarCipher cipher(key);

    std::error_code ec;
    if (!fs::exists(target, ec)) {
        Logger::error("path does not exist: " + target.string());
        return 1;
    }

    // ---- Single-file mode (no fork) ---------------------------------------
    if (fs::is_regular_file(target, ec)) {
        fs::path out;
        std::string err;
        if (!outputPathFor(target, mode, out, err) ||
            !transformFile({target, out, mode}, cipher, err)) {
            Logger::error(err);
            return 1;
        }
        Logger::info(std::string(mode == Mode::Encrypt ? "encrypted " : "decrypted ") +
                     target.string() + " -> " + out.string());
        return 0;
    }

    // ---- Directory mode: enqueue every file, run in parallel --------------
    if (!fs::is_directory(target, ec)) {
        Logger::error("not a file or directory: " + target.string());
        return 1;
    }

    ProcessManagement pm(cipher);
    for (const Task& t : collectTasks(target, mode)) pm.enqueue(t);

    std::size_t total = pm.pending();
    Logger::info("queued " + std::to_string(total) + " file(s), workers=" + std::to_string(jobs));
    Summary s = pm.run(jobs);
    Logger::info("done: " + std::to_string(s.succeeded) + " ok, " + std::to_string(s.failed) + " failed");
    return s.failed == 0 ? 0 : 2;
}
