#pragma once
#include <map>
#include <string>

namespace fenc {

// Minimal .env parser: KEY=VALUE lines, '#' comments, optional quotes.
class EnvLoader {
public:
    static std::map<std::string, std::string> load(const std::string& path);
    // Reads an integer key (e.g. ENCRYPTION_KEY) from the env map. Throws on failure.
    static int getIntKey(const std::map<std::string, std::string>& env,
                         const std::string& name);
};

}  // namespace fenc
