#pragma once
#include <3ds.h>
#include <string>

#define TIMEZONE_OFFSET (6 * 3600)

namespace totp {
    u32 get_seconds_remaining();

    u32 get_code(const std::string &key);
}
