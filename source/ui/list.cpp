#include <3ds.h>
#include <citro2d.h>
#include <cstdio>
#include <optional>
#include <sstream>
#include <iomanip>
#include "otp_entry.h"
#include "ui/list.h"
#include "scene.h"
#include "b32decode.h"

extern "C" {
    #include "hmac.h"
}

#define WIDTH 400
#define HEIGHT 240
#define SCREEN_SIZE WIDTH *HEIGHT * 2

std::string base = "JBSWY3DPEHPK3PXP";

namespace totp {

list::list() {
    code_buf = C2D_TextBufNew(64);
}
list::~list() {
    C2D_TextBufDelete(code_buf);
}

std::optional<scene_type> list::update() {
    std::optional<scene_type> result;
    u32 kDown = hidKeysDown();
    if(kDown & KEY_START) {
        printf("Exit\n");
        return scene_type::EXIT;
    }

    otp_entry entry(base);

    u32 code = entry.get_code();

    float percent = static_cast<float>(entry.remaining_time()) / 30;

    std::stringstream code_str;
    code_str << std::setw(6) << std::setfill('0') << code;

    C2D_TextBufClear(code_buf);
    C2D_Text code_text;
    C2D_TextParse(&code_text, code_buf, code_str.str().data());
    C2D_TextOptimize(&code_text);

    C2D_DrawRectangle(WIDTH / 2.f - 50, HEIGHT / 2.f, 0, 100, 2, clrBlack,
                      clrBlack, clrBlack, clrBlack);
    C2D_DrawRectSolid(WIDTH / 2.f - 50, HEIGHT / 2.f, 0, percent * 100, 2,
                      clrBlue);
    C2D_DrawText(&code_text, C2D_WithColor, WIDTH / 2.f - code_text.width / 2,
                 HEIGHT / 2.f - 32, 0, 1, 1, clrBlack);

    return result;
}

}
