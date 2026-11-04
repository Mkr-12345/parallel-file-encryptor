#pragma once
#include <cstddef>
#include "Task.hpp"

namespace fenc {

// Byte-wise Caesar cipher (shift modulo 256), so it works on any binary file.
// NOTE: educational only - NOT secure. See README.
class CaesarCipher {
public:
    explicit CaesarCipher(int key);
    void apply(char* buf, std::size_t n, Mode mode) const;

private:
    unsigned char shift_;
};

}  // namespace fenc
