#pragma once
#include <filesystem>
#include <string>
#include "Cipher.hpp"
#include "Task.hpp"

namespace fenc {

// Streams `task.input` -> `task.output` in chunks, applying the cipher.
// Returns false and fills `err` on failure (partial output is removed).
bool transformFile(const Task& task, const CaesarCipher& cipher, std::string& err);

// Naming convention: encrypt  a.txt     -> a.txt.enc
//                    decrypt  a.txt.enc -> a.txt.dec
// Returns false if the path is not valid for the mode.
bool outputPathFor(const std::filesystem::path& in, Mode mode,
                   std::filesystem::path& out, std::string& err);

}  // namespace fenc
