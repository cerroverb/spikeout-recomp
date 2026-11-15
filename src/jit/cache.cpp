#include "jit/cache.hpp"

#include <monocypher.h>

#include <cstdint>
#include <cstdio>
#include <fstream>
#include <random>
#include <sstream>

namespace jit::cache {

namespace {

constexpr const char* kTemporaryEnd = ".tmp";

std::filesystem::path part_path(const std::filesystem::path& dir, std::size_t i) {
    return dir / ("part" + std::to_string(i) + ".o");
}

std::string hash_hex(const std::vector<char>& bytes) {
    uint8_t hash[32];
    crypto_blake2b(hash, sizeof hash, reinterpret_cast<const uint8_t*>(bytes.data()), bytes.size());
    std::string hex(2 * sizeof hash, '0');
    for (std::size_t i = 0; i < sizeof hash; ++i) std::snprintf(&hex[2 * i], 3, "%02x", hash[i]);
    return hex;
}

std::vector<char> read_file(const std::filesystem::path& p) {
    std::ifstream in(p, std::ios::binary);
    if (!in) return {};
    return {std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>()};
}

// Written beside its final name under a name no other instance uses, then renamed over it (a rename within a folder
// replaces the file whole: a reader sees the old file or the new one).
bool write_whole(const std::filesystem::path& to, const char* bytes, std::size_t size) {
    static thread_local std::mt19937_64 random{std::random_device{}()};
    char suffix[24];
    std::snprintf(suffix, sizeof suffix, ".%016llx", static_cast<unsigned long long>(random()));
    std::filesystem::path temporary = to; // (part3.o.<random>.tmp)
    temporary += suffix;
    temporary += kTemporaryEnd;
    {
        std::ofstream out(temporary, std::ios::binary);
        out.write(bytes, static_cast<std::streamsize>(size));
        out.close();
        if (!out) {
            std::error_code ec;
            std::filesystem::remove(temporary, ec);
            return false;
        }
    }
    std::error_code ec;
    std::filesystem::rename(temporary, to, ec);
    if (ec) std::filesystem::remove(temporary, ec);
    return !ec;
}

} // namespace

Loaded read(const std::filesystem::path& dir, std::string_view version, std::size_t count) {
    std::ifstream complete(dir / "complete");
    if (!complete) return {};
    std::string line;
    if (!std::getline(complete, line) || line != version) return {{}, "made by another version"};
    Loaded loaded;
    for (std::size_t i = 0; i < count; ++i) {
        std::string name, hash;
        std::size_t size = 0;
        if (!std::getline(complete, line) || !(std::istringstream(line) >> name >> size >> hash) ||
            name != part_path(dir, i).filename().string())
            return {{}, "its list of parts is incomplete"};
        std::vector<char> object = read_file(part_path(dir, i));
        if (object.size() != size || hash_hex(object) != hash) return {{}, name + " doesn't match its hash"};
        loaded.objects.push_back(std::move(object));
    }
    return loaded;
}

bool write(const std::filesystem::path& dir, std::string_view version, const Objects& objects) {
    std::error_code ec;
    std::filesystem::create_directories(dir, ec);
    if (ec) return false;
    std::string list = std::string(version) + "\n";
    for (std::size_t i = 0; i < objects.size(); ++i) {
        const std::filesystem::path path = part_path(dir, i);
        if (!write_whole(path, objects[i].data(), objects[i].size())) return false;
        list += path.filename().string() + " " + std::to_string(objects[i].size()) + " " + hash_hex(objects[i]) + "\n";
    }
    return write_whole(dir / "complete", list.data(), list.size());
}

bool is_cache_file(const std::string& name) {
    std::filesystem::path final_name = name;
    if (final_name.extension() == kTemporaryEnd) final_name = final_name.stem().stem(); // (its random suffix too)
    const std::string n = final_name.string();
    return n == "complete" || (n.rfind("part", 0) == 0 && final_name.extension() == ".o");
}

} // namespace jit::cache
