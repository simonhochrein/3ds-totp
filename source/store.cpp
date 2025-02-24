#include "store.h"
#include <algorithm>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <sstream>
#include "utils.h"
#include <ranges>

struct store_entry {
    short name_size;
    char name[64];
    short username_size;
    char username[64];
    short key_size;
    char key[128];
};

struct store_headers {
    char magic[4];
    short count;
};

namespace totp {
    store::store() {
        load_entries();
    }

    void store::add_entry(const otp_entry &entry) {
        entries.push_back(entry);
        save_entries();
    }

    void store::remove_entry(int idx) {
        entries.erase(entries.begin() + idx);
        save_entries();
    }

    std::vector<otp_entry> &store::get_entries() const {
        return const_cast<std::vector<otp_entry> &>(entries);
    }

    void store::save_entries() {
        // Implementation to save entries to a file or database
        std::ofstream file("store.bin", std::ios::out | std::ios::binary);
        store_headers headers{{'T', 'O', 'T', 'P'}, static_cast<short>(entries.size())};
        file.write(reinterpret_cast<char *>(&headers), sizeof(store_headers));
        for (const auto &entry: entries) {
            store_entry out_entry{};
            out_entry.name_size = entry.get_label().size();
            out_entry.username_size = entry.get_username().size();
            out_entry.key_size = entry.get_key().size();

            entry.get_label().copy(out_entry.name, out_entry.name_size);
            entry.get_username().copy(out_entry.username, out_entry.username_size);
            entry.get_key().copy(out_entry.key, out_entry.key_size);
            file.write(reinterpret_cast<char *>(&out_entry), sizeof(store_entry));
        }
        file.close();
    }

    void store::load_entries() {
        // Implementation to load entries from a file or database
        std::ifstream file("store.bin", std::ios_base::in | std::ios_base::binary);
        if (!file.is_open()) {
            std::cout << "Failed to open store.bin" << std::endl;
            return;
        }

        store_headers headers{};
        file.read(reinterpret_cast<char *>(&headers), sizeof(store_headers));

        std::cout.write(headers.magic, 4);
        std::cout << std::endl << headers.count << std::endl;

        entries.clear();
        for (int i = 0; i < headers.count; ++i) {
            store_entry entry{};
            file.read(reinterpret_cast<char *>(&entry), sizeof(store_entry));
            entries.emplace_back(std::string(std::string_view(entry.name, entry.name_size)),
                                 std::string(std::string_view(entry.username, entry.username_size)),
                                 std::string(std::string_view(entry.key, entry.key_size)));
        }
        file.close();
    }

    void store::reset_entries() {
        std::cout << "Resetting entries..." << std::endl;
        entries.clear();
        save_entries();
    }
}
