// REF: https://github.com/mjg59/tpmtotp/blob/master/base32.c

#include "strings.h"
#include <stddef.h>
#include <stdio.h>
static unsigned char shift_right(unsigned char byte, signed char offset) {
  if (offset > 0)
    return byte >> offset;
  else
    return byte << -offset;
}

static unsigned char shift_left(unsigned char byte, signed char offset) {
  return shift_right(byte, -offset);
}

static int decode_char(unsigned char c) {
  signed char retval = -1;

  if (c >= 'A' && c <= 'Z')
    retval = c - 'A';
  if (c >= '2' && c <= '7')
    retval = c - '2' + 26;

  return retval;
}

static int get_octet(int block) { return (block * 5) / 8; }

static int get_offset(int block) { return (8 - 5 - (5 * block) % 8); }

static int decode_sequence(const unsigned char *coded, unsigned char *plain) {
  plain[0] = 0;
  for (int block = 0; block < 8; block++) {
    int offset = get_offset(block);
    int octet = get_octet(block);
    int c = decode_char(coded[block]);

    if (c < 0)
      return octet;

    plain[octet] |= shift_left(c, offset);
    if (offset < 0)
      plain[octet + 1] = shift_left(c, 8 + offset);
  }
  return 5;
}

size_t base32_decode(const unsigned char *coded, unsigned char *plain) {
  size_t written = 0;
  for (size_t i = 0, j = 0;; i += 8, j += 5) {
    int n = decode_sequence(&coded[i], &plain[j]);
    written += n;
    if (n < 5)
      return written;
  }
}
