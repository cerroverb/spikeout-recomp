// Save states sent over the network, compressed with zstd: a board's 29 MB
// state is 3-5 MB so (docs/netplay.md).
#pragma once

#include <cstdint>
#include <optional>
#include <span>
#include <vector>

namespace net {

std::vector<uint8_t> compress(std::span<const uint8_t> data, int level = 3);
// Empty if `data` isn't zstd's (or is damaged), or says it unpacks to more than `max_bytes` (what the caller can
// expect: the size is the sender's to say).
std::optional<std::vector<uint8_t>> decompress(std::span<const uint8_t> data, std::size_t max_bytes);

} // namespace net
