#include "store.h"
#include <algorithm>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <sstream>

namespace totp {
    store::store() {
        load_entries();
    }

    void store::add_entry(const otp_entry& entry) {
        entries.push_back(entry);
        save_entries();
    }

    void store::remove_entry(const otp_entry& entry) {
        entries.erase(std::find(entries.begin(), entries.end(), entry));
        save_entries();
    }

    std::vector<otp_entry>& store::get_entries() const {
        return const_cast<std::vector<otp_entry>&>(entries);
    }

    void store::save_entries() {
        // Implementation to save entries to a file or database
        std::ofstream file("store.txt", std::ios::out | std::ios::binary);
        for (const auto& entry : entries) {
            file << entry << std::endl;
        }
        file.close();
    }

    void store::load_entries() {
        // Implementation to load entries from a file or database
        std::ifstream file("store.txt", std::ios_base::in | std::ios_base::binary);
        if (!file.is_open()) {
            std::cout << "Failed to open store.txt" << std::endl;
            return;
        }

        std::string line;
        while (std::getline(file, line)) {
            entries.emplace_back(line);
        }
    }
}
