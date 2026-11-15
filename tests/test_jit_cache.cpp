#include "test.hpp"

#include "jit/cache.hpp"

#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

namespace fs = std::filesystem;

namespace {

constexpr const char* kVersion = "spikeout-jit test";

fs::path fresh_dir(const char* name) {
    const fs::path dir = fs::temp_directory_path() / name;
    fs::remove_all(dir);
    return dir;
}

jit::cache::Objects some_objects() {
    jit::cache::Objects objects;
    for (char c = 'a'; c < 'e'; ++c) objects.push_back(std::vector<char>(1000 + c, c));
    return objects;
}

} // namespace

TEST_CASE("jit cache: parts written are read back as they were") {
    const fs::path dir = fresh_dir("spikeout-jit-cache-round-trip");
    const jit::cache::Objects objects = some_objects();
    CHECK(jit::cache::write(dir, kVersion, objects));
    const jit::cache::Loaded loaded = jit::cache::read(dir, kVersion, objects.size());
    CHECK(loaded.objects == objects);
    CHECK(loaded.problem.empty());
    for (const auto& f : fs::directory_iterator(dir)) // (no temporary file left)
        CHECK_EQ(f.path().extension() == ".tmp", false);
    fs::remove_all(dir);
}

TEST_CASE("jit cache: nothing kept, nothing loaded") {
    const jit::cache::Loaded loaded = jit::cache::read(fresh_dir("spikeout-jit-cache-none"), kVersion, 4);
    CHECK(loaded.objects.empty());
    CHECK(loaded.problem.empty());
}

TEST_CASE("jit cache: a truncated part is refused") {
    const fs::path dir = fresh_dir("spikeout-jit-cache-truncated");
    CHECK(jit::cache::write(dir, kVersion, some_objects()));
    fs::resize_file(dir / "part2.o", 10);
    const jit::cache::Loaded loaded = jit::cache::read(dir, kVersion, 4);
    CHECK(loaded.objects.empty());
    CHECK_EQ(loaded.problem, std::string("part2.o doesn't match its hash"));
    fs::remove_all(dir);
}

TEST_CASE("jit cache: a changed byte in a part is refused") {
    const fs::path dir = fresh_dir("spikeout-jit-cache-changed");
    CHECK(jit::cache::write(dir, kVersion, some_objects()));
    {
        std::fstream f(dir / "part1.o", std::ios::in | std::ios::out | std::ios::binary);
        f.seekp(500);
        f.put('x');
    }
    const jit::cache::Loaded loaded = jit::cache::read(dir, kVersion, 4);
    CHECK(loaded.objects.empty());
    CHECK_EQ(loaded.problem, std::string("part1.o doesn't match its hash"));
    fs::remove_all(dir);
}

TEST_CASE("jit cache: another version's, or an old list without hashes, is refused") {
    const fs::path dir = fresh_dir("spikeout-jit-cache-version");
    CHECK(jit::cache::write(dir, kVersion, some_objects()));
    CHECK_EQ(jit::cache::read(dir, "spikeout-jit other", 4).problem, std::string("made by another version"));
    std::ofstream(dir / "complete") << kVersion << "\n"; // (as the cache was written before the hashes)
    CHECK_EQ(jit::cache::read(dir, kVersion, 4).problem, std::string("its list of parts is incomplete"));
    CHECK(jit::cache::read(dir, kVersion, 4).objects.empty());
    fs::remove_all(dir);
}

TEST_CASE("jit cache: a part missing is refused") {
    const fs::path dir = fresh_dir("spikeout-jit-cache-missing");
    CHECK(jit::cache::write(dir, kVersion, some_objects()));
    fs::remove(dir / "part3.o");
    CHECK(jit::cache::read(dir, kVersion, 4).objects.empty());
    fs::remove_all(dir);
}

TEST_CASE("jit cache: its own files, and the temporary files of a stopped instance, are known") {
    CHECK(jit::cache::is_cache_file("complete"));
    CHECK(jit::cache::is_cache_file("part0.o"));
    CHECK(jit::cache::is_cache_file("part48.o"));
    CHECK(jit::cache::is_cache_file("part7.o.0123456789abcdef.tmp"));
    CHECK(jit::cache::is_cache_file("complete.0123456789abcdef.tmp"));
    CHECK(!jit::cache::is_cache_file("settings.ini"));
    CHECK(!jit::cache::is_cache_file("part0.txt"));
    CHECK(!jit::cache::is_cache_file("notes.tmp"));
}
