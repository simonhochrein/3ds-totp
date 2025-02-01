#pragma once

#include <stddef.h>

size_t base32_decode(const unsigned char *coded, unsigned char *plain);
