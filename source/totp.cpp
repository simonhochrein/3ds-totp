#include "totp.h"
#include "b32decode.h"
extern "C" {
#include "hmac.h"
}
#include <cmath>
#include <iostream>

namespace totp {

u32 get_seconds_remaining() {
    long long unixTime = time(nullptr);
    unixTime += TIMEZONE_OFFSET;

    return 30 - (unixTime % 30);
}

u32 get_code(const std::string &key) {
  long long unixTime = time(nullptr);
  unixTime += TIMEZONE_OFFSET;

  const long long message = unixTime / 30;

  const uint8_t message_str[] = {0,
                                 0,
                                 0,
                                 0,
                                 static_cast<uint8_t>((message >> 24) & 0xFF),
                                 static_cast<uint8_t>((message >> 16) & 0xFF),
                                 static_cast<uint8_t>((message >> 8) & 0xFF),
                                 static_cast<uint8_t>((message >> 0) & 0xFF)};

  uint8_t mac[20] = {};

  hmac_sha1(reinterpret_cast<const uint8_t*>(key.c_str()), key.size(), message_str, 8, mac);

  const uint8_t offset = mac[19] & 0xf;
  const int bits = ((mac[offset] & 0x7f) << 24) | ((mac[offset + 1] & 0xff) << 16) |
             ((mac[offset + 2] & 0xff) << 8) | ((mac[offset + 3] & 0xff));

  return bits % 1000000;
}
} // namespace totp
