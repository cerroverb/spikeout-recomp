// libcurl's one-time set-up, shared by the lobby, the update check and bug reports, which start on different
// threads: curl_global_init must not run on two at once. Never cleaned up: a detached question may outlive main.
#pragma once

#include <curl/curl.h>

namespace net {

inline void init_curl() {
    // (a failure is not kept: curl_easy_init then tries again itself, and each caller reports its null)
    static const CURLcode done = curl_global_init(CURL_GLOBAL_DEFAULT);
    (void)done;
}

} // namespace net
