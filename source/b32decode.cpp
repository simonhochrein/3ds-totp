#include "b32decode.h"

std::string base32_decode(const std::string &coded) {
  std::string input = coded;
  std::string decoded_bytes;
  int buffer = 0;
  int bit_count = 0;

  for (const char c : input) {
    int value;
    if (c >= 'A' && c <= 'Z') {
      value = c - 'A';
    } else if (c >= '2' && c <= '7') {
      value = c - '2' + 26;
    } else {
        printf("Failed to decode\n");
        return {};
    }

    buffer = (buffer << 5) | value;
    bit_count += 5;

    if (bit_count >= 8) {
      decoded_bytes.push_back((buffer >> (bit_count - 8)) & 0xFF);
      bit_count -= 8;
      buffer &= 0xFF >> (8 - bit_count);
    }
  }

  return decoded_bytes;
}
