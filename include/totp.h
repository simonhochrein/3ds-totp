#pragma once
#include <3ds.h>
#include <string>

#define TIMEZONE_OFFSET (6 * 3600)

namespace totp {
    struct totp_key {
    public:
        unsigned char key[32];
        size_t len;
        static totp_key parse(std::string key);
    };

    const u32 get_seconds_remaining();
    const u32 get_code(const totp_key &key);
}
