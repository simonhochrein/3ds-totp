#pragma once
#include "totp.h"
#include <3ds.h>
#include <cstring>
#include <string>

namespace totp {
class otp_entry {
public:
    explicit otp_entry(std::string key);

    [[nodiscard]] u32 get_code() const;
    [[nodiscard]] u32 remaining_time() const;

    bool operator==(const otp_entry& other) const {
        return other.key == key;
    }

    friend std::ostream& operator <<(std::ostream& os, const otp_entry& entry) {
        os << entry.key;
        return os;
    }
private:
    std::string key;
};
}
