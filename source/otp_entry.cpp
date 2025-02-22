#include "otp_entry.h"

#include <utility>
#include "totp.h"

namespace totp {
    otp_entry::otp_entry(std::string label, std::string username, std::string key): label(std::move(label)),
        username(std::move(username)),
        key(std::move(key)) {
    }

    u32 otp_entry::get_code() const {
        return totp::get_code(key);
    }

    u32 otp_entry::remaining_time() const {
        return get_seconds_remaining();
    }

    std::string otp_entry::get_label() const {
        return label;
    }

    std::string otp_entry::get_username() const {
        return username;
    }

    std::string otp_entry::get_key() const {
        return key;
    }

    bool otp_entry::operator==(const otp_entry &other) const {
        return other.key == key;
    }

    std::ostream &operator<<(std::ostream &os, const otp_entry &entry) {
        os << entry.label;
        os << std::string(":");
        os << entry.username;
        os << std::string(":");
        os << entry.key;
        return os;
    }
}
