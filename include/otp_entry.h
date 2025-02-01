#pragma once
#include "totp.h"
#include <3ds.h>
#include <string>
namespace totp {
class otp_entry {
public:
    otp_entry(std::string key);

    const u32 get_code();
    const u32 remaining_time();
private:
    totp_key key;
};
}
