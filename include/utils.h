#pragma once

namespace totp {
    template<typename Out>
    void split(const std::string&s, const char delim, Out result) {
        std::istringstream iss(s);
        std::string item;
        while (std::getline(iss, item, delim)) {
            *result++ = item;
        }
    }

    inline std::vector<std::string> split(const std::string&s, const char delim) {
        std::vector<std::string> elems;
        split(s, delim, std::back_inserter(elems));
        return elems;
    }
}