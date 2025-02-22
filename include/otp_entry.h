#pragma once
#include "totp.h"
#include <3ds.h>
#include <cstring>
#include <string>

namespace totp {
class otp_entry {
public:
    explicit otp_entry(std::string label, std::string username, std::string key);

    [[nodiscard]] u32 get_code() const;
    [[nodiscard]] u32 remaining_time() const;

    std::string get_label() const;
    std::string get_username() const;
    std::string get_key() const;


    bool operator==(const otp_entry& other) const;

    friend std::ostream& operator <<(std::ostream& os, const otp_entry& entry);

private:
    std::string label;
    std::string username;
    std::string key;
};
}
