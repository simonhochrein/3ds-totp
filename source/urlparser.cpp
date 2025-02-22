#include "urlparser.h"

#include <b32decode.h>
#include <string_view>
#include <uri.hpp>

namespace totp {
    urlparser::urlparser(std::string_view url) {
        FIX8::uri uri(url);
        if (uri.get_scheme() != "otpauth") {
            std::cout << "Invalid Scheme: " << uri.get_scheme() << std::endl;
            return;
        }
        if (uri.get_authority() != "totp") {
            std::cout << "Invalid Authority: " << uri.get_authority() << std::endl;
            return;
        }

        auto path = uri.get_path().substr(1);

        if (const auto colon = path.find(':'); colon != std::string::npos) {
            label = url_decode(path.substr(0, colon));
            username = path.substr(colon + 1);
        } else {
            label = url_decode(path);
            username = "";
        }

        auto query = uri.get_query();
        size_t pos_start = 0, pos_end = 0;
        while ((pos_end = query.find('&', pos_start)) != std::string::npos) {
            auto pair = query.substr(pos_start, pos_end - pos_start);
            if (pair.find("secret=") != std::string::npos) {
                secret = pair.substr(pair.find('=') + 1);
                break;
            }
            pos_start = pos_end + 1;
        }
        valid = true;
    }

    bool urlparser::is_valid() const {
        return valid;
    }

    otp_entry urlparser::get_entry() const {
        return otp_entry{label, username, base32_decode(secret)};
    }


    std::string urlparser::url_decode(std::string_view in) {
        std::string out;
        for (int i = 0; i < in.length(); i++) {
            if (in[i] == '%') {
                if (i + 2 < in.length()) {
                    std::string s(in.substr(i + 1, 2));
                    out += static_cast<char>(std::stoi(s, nullptr, 16));
                    i += 2;
                    continue;
                }
            }
            out += in[i];
        }
        return out;
    }
}
