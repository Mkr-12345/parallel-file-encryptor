#include "FileIO.hpp"

#include <array>
#include <fstream>

namespace fenc {

namespace fs = std::filesystem;

bool outputPathFor(const fs::path& in, Mode mode, fs::path& out, std::string& err) {
    if (mode == Mode::Encrypt) {
        if (in.extension() == ".enc") {
            err = "already encrypted (.enc): " + in.string();
            return false;
        }
        out = in;
        out += ".enc";
    } else {
        if (in.extension() != ".enc") {
            err = "not an .enc file: " + in.string();
            return false;
        }
        out = in;
        out.replace_extension(".dec");
    }
    return true;
}

static void removeQuietly(const fs::path& p) {
    std::error_code ec;
    fs::remove(p, ec);
}

bool transformFile(const Task& task, const CaesarCipher& cipher, std::string& err) {
    std::ifstream in(task.input, std::ios::binary);
    if (!in) {
        err = "cannot open input: " + task.input.string();
        return false;
    }
    std::ofstream out(task.output, std::ios::binary | std::ios::trunc);
    if (!out) {
        err = "cannot open output: " + task.output.string();
        return false;
    }

    std::array<char, 64 * 1024> buf;
    while (in) {
        in.read(buf.data(), static_cast<std::streamsize>(buf.size()));
        std::streamsize n = in.gcount();
        if (n <= 0) break;
        cipher.apply(buf.data(), static_cast<std::size_t>(n), task.mode);
        out.write(buf.data(), n);
        if (!out) {
            err = "write failed: " + task.output.string();
            out.close();
            removeQuietly(task.output);
            return false;
        }
    }
    if (in.bad()) {
        err = "read failed: " + task.input.string();
        out.close();
        removeQuietly(task.output);
        return false;
    }
    return true;
}

}  // namespace fenc
