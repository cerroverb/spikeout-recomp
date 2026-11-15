// Bytes from the system's own secure source (BCryptGenRandom, arc4random_buf, getrandom), for secrets: a report's
// lock (tools/report_lock.hpp) and each seat's secret in network play (src/net/session.cpp).
#pragma once

#include <cstddef>
#include <cstdint>

namespace net {

// False if it gave none.
bool random_bytes(uint8_t* out, std::size_t n);

} // namespace net
