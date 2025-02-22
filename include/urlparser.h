#pragma once

#include <otp_entry.h>
#include <string>

namespace totp {

class urlparser {
public:
    explicit urlparser(std::string_view url);

    bool is_valid() const;

    otp_entry get_entry() const;
private:
    std::string label;
    std::string username;
    std::string secret;
    bool valid = false;

    static std::string url_decode(std::string_view in);
};

}
