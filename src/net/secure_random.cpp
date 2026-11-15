#include "net/secure_random.hpp"

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <bcrypt.h>
#elif defined(__APPLE__)
#include <stdlib.h> // (arc4random_buf)
#else
#include <sys/random.h>
#endif

namespace net {

bool random_bytes(uint8_t* out, std::size_t n) {
#ifdef _WIN32
    return BCryptGenRandom(nullptr, out, static_cast<ULONG>(n), BCRYPT_USE_SYSTEM_PREFERRED_RNG) == 0;
#elif defined(__APPLE__)
    arc4random_buf(out, n);
    return true;
#else
    while (n) {
        const ssize_t got = getrandom(out, n, 0);
        if (got <= 0) return false;
        out += got, n -= static_cast<std::size_t>(got);
    }
    return true;
#endif
}

} // namespace net
