#include "otp_entry.h"
#include "totp.h"

namespace totp {
    otp_entry::otp_entry(std::string key): key(key) {}

    u32 otp_entry::get_code() const {
        return totp::get_code(key);
    }

    u32 otp_entry::remaining_time() const {
        return get_seconds_remaining();
    }
}
