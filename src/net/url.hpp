// URLs handed to the system to open (SDL_OpenURL): pages named by GitHub's answers are checked first.
#pragma once

#include <cctype>
#include <cstddef>
#include <string_view>

namespace net {

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
