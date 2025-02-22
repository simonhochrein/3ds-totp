#pragma once
#include "otp_entry.h"
#include <vector>

namespace totp {
    class store {
        public:
            store();
            void add_entry(const otp_entry& entry);
            void remove_entry(int idx);
            void save_entries();
            void load_entries();
            void reset_entries();
            [[nodiscard]] std::vector<otp_entry>& get_entries() const;


        private:
            std::vector<otp_entry> entries;
    };
}
