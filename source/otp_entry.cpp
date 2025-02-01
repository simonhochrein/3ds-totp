#include "otp_entry.h"
#include "totp.h"

namespace totp {
    otp_entry::otp_entry(std::string key): key(totp_key::parse(key)) {}

    const u32 otp_entry::get_code() {
        return totp::get_code(key);
    }

    const u32 otp_entry::remaining_time() {
        return totp::get_seconds_remaining();
    }
}
