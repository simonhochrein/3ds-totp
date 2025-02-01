#pragma once
#include <3ds.h>
#include <citro2d.h>
#include "otp_entry.h"
#include "scene.h"

namespace totp {
class list : public scene {
public:
  list();

  [[nodiscard]] std::optional<scene_type> update() final;
private:
  C2D_TextBuf code_buf;
  uint8_t key[64] = {0};
  size_t key_len;

  u32 clrBlack = C2D_Color32(0x00, 0x00, 0x00, 0xFF);
  u32 clrBlue = C2D_Color32(0x00, 0x00, 0xFF, 0xFF);
  u32 clrClear = C2D_Color32(0xFF, 0xFF, 0xFF, 0xFF);

protected:
  ~list();
};
} // namespace totp
