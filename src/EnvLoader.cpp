#include "EnvLoader.hpp"

#include <fstream>
#include <stdexcept>

namespace fenc {

static std::string trim(const std::string& s) {
    const char* ws = " \t\r\n";
    auto b = s.find_first_not_of(ws);
    if (b == std::string::npos) return "";
    auto e = s.find_last_not_of(ws);
    return s.substr(b, e - b + 1);
}

std::map<std::string, std::string> EnvLoader::load(const std::string& path) {
    std::ifstream in(path);
    if (!in) throw std::runtime_error("cannot open env file: " + path);

    std::map<std::string, std::string> env;
    std::string line;
    while (std::getline(in, line)) {
        line = trim(line);
        if (line.empty() || line[0] == '#') continue;
        auto eq = line.find('=');
        if (eq == std::string::npos) continue;
        std::string key = trim(line.substr(0, eq));
        std::string val = trim(line.substr(eq + 1));
        if (val.size() >= 2 && ((val.front() == '"' && val.back() == '"') ||
                                (val.front() == '\'' && val.back() == '\'')))
            val = val.substr(1, val.size() - 2);
        env[key] = val;
    }
    return env;
}

int EnvLoader::getIntKey(const std::map<std::string, std::string>& env,
                         const std::string& name) {
    auto it = env.find(name);
    if (it == env.end()) throw std::runtime_error(name + " not found in env file");
    try {
        std::size_t pos = 0;
        int v = std::stoi(it->second, &pos);
        if (pos != it->second.size()) throw std::invalid_argument("trailing chars");
        return v;
    } catch (const std::exception&) {
        throw std::runtime_error(name + " must be an integer");
    }
}

}  // namespace fenc
