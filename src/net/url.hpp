// URLs handed to the system to open (SDL_OpenURL): pages named by GitHub's answers are checked first, and folders
// are made into file:// URLs.
#pragma once

#include <cctype>
#include <cstddef>
#include <cstdio>
#include <string>
#include <string_view>

namespace net {

// A folder's file:// URL: `path` (UTF-8, Windows' backslashes or not) with every byte but letters, digits and
// "/:._-~" percent-encoded (spaces, '#', '%', non-ASCII names).
inline std::string file_url(std::string_view path) {
    std::string url = "file://";
    if (!path.empty() && path[0] != '/') url += '/'; // (C:/...: file:///C:/...)
    for (const char c : path) {
        const auto u = static_cast<unsigned char>(c);
        if (std::isalnum(u) || std::string_view("/:._-~").find(c) != std::string_view::npos) url += c;
        else if (c == '\\') url += '/';
        else {
            char hex[4];
            std::snprintf(hex, sizeof hex, "%%%02X", u);
            url += hex;
        }
    }
    return url;
}

// Whether `url` is an https:// address (the scheme in any case) with something after it and no spaces or control
// characters: the only kind a page from the network may name for the system to open.
inline bool is_https_url(std::string_view url) {
    constexpr std::string_view scheme = "https://";
    if (url.size() <= scheme.size()) return false;
    for (std::size_t i = 0; i < scheme.size(); ++i)
        if (std::tolower(static_cast<unsigned char>(url[i])) != scheme[i]) return false;
    for (const char c : url)
        if (static_cast<unsigned char>(c) <= 0x20 || c == 0x7F) return false;
    return true;
}

} // namespace net
