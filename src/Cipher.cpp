#include "Cipher.hpp"

namespace fenc {

CaesarCipher::CaesarCipher(int key)
    : shift_(static_cast<unsigned char>(((key % 256) + 256) % 256)) {}

void CaesarCipher::apply(char* buf, std::size_t n, Mode mode) const {
    for (std::size_t i = 0; i < n; ++i) {
        unsigned char b = static_cast<unsigned char>(buf[i]);
        b = (mode == Mode::Encrypt) ? static_cast<unsigned char>(b + shift_)
                                    : static_cast<unsigned char>(b - shift_);
        buf[i] = static_cast<char>(b);
    }
}

}  // namespace fenc
