#pragma once
#include <filesystem>

namespace fenc {

enum class Mode { Encrypt, Decrypt };

struct Task {
    std::filesystem::path input;
    std::filesystem::path output;
    Mode mode;
};

}  // namespace fenc
