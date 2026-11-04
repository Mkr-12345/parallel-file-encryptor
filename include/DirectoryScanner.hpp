#pragma once
#include <filesystem>
#include <vector>
#include "Task.hpp"

namespace fenc {

// Recursively walks `root` (via std::filesystem) and builds one Task per eligible file.
std::vector<Task> collectTasks(const std::filesystem::path& root, Mode mode);

}  // namespace fenc
