#include "totp.h"
#include "b32decode.h"
extern "C" {
#include "hmac.h"
}
#include <cmath>

namespace totp {
totp_key totp_key::parse(std::string key) {
  totp_key result;
  result.len = static_cast<uint32_t>(
      base32_decode((unsigned char *)key.data(), result.key));
  return result;
}
const u32 get_seconds_remaining() {
    int unixTime = time(NULL);
    unixTime += TIMEZONE_OFFSET;

    return 30 - (unixTime % 30);
}
const u32 get_code(const totp_key &key) {
  int unixTime = time(NULL);
  unixTime += TIMEZONE_OFFSET;

  int message = unixTime / 30;

  const uint8_t message_str[] = {0,
                                 0,
                                 0,
                                 0,
                                 static_cast<uint8_t>((message >> 24) & 0xFF),
                                 static_cast<uint8_t>((message >> 16) & 0xFF),
                                 static_cast<uint8_t>((message >> 8) & 0xFF),
                                 static_cast<uint8_t>((message >> 0) & 0xFF)};

  uint8_t mac[20] = {0};

  hmac_sha1(key.key, key.len, message_str, 8, mac);

  // print_hex_bytes(mac, sizeof(mac));

  uint8_t offset = mac[19] & 0xf;
  int bits = ((mac[offset] & 0x7f) << 24) | ((mac[offset + 1] & 0xff) << 16) |
             ((mac[offset + 2] & 0xff) << 8) | ((mac[offset + 3] & 0xff));

  int code = bits % 1000000;

  return code;
}
} // namespace totp
