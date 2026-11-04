#include "DirectoryScanner.hpp"

#include "FileIO.hpp"
#include "Logger.hpp"

namespace fenc {

namespace fs = std::filesystem;

std::vector<Task> collectTasks(const fs::path& root, Mode mode) {
    std::vector<Task> tasks;
    std::error_code ec;
    fs::recursive_directory_iterator it(root, fs::directory_options::skip_permission_denied, ec), end;
    if (ec) {
        Logger::error("cannot read directory " + root.string() + ": " + ec.message());
        return tasks;
    }
    for (; it != end; it.increment(ec)) {
        if (ec) break;
        if (!it->is_regular_file(ec)) continue;
        fs::path out;
        std::string err;
        if (!outputPathFor(it->path(), mode, out, err)) continue;  // skip ineligible files
        tasks.push_back({it->path(), out, mode});
    }
    return tasks;
}

}  // namespace fenc
