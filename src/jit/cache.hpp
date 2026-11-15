// The JIT's translation kept on disk (src/jit/jit.cpp): one object file a part (part<i>.o) and `complete`, written
// last, naming the translator's version and each part's size and BLAKE2b hash. Every file is written under a
// temporary name and renamed into place, so another instance starting at the same moment never reads half a file,
// and a part is loaded as native code only when it matches its hash in `complete`.
#pragma once

#include <filesystem>
#include <string>
#include <string_view>
#include <vector>

namespace jit::cache {

using Objects = std::vector<std::vector<char>>;

struct Loaded {
    Objects objects;     // empty: nothing usable, translate again
    std::string problem; // why not (empty when there was no `complete` at all, or the objects are good)
};

// The `count` parts in `dir`, when `complete` there names `version` and every part matches its size and hash.
Loaded read(const std::filesystem::path& dir, std::string_view version, std::size_t count);

// The parts, then `complete`; false (and nothing half-written left under a final name) when a file can't be written.
bool write(const std::filesystem::path& dir, std::string_view version, const Objects& objects);

// A file this cache writes, or one of its temporary files a stopped instance left behind (for pruning old folders).
bool is_cache_file(const std::string& name);

} // namespace jit::cache
