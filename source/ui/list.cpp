#include <3ds.h>
#include "ui/list.h"
#include "b32decode.h"
#include "scene.h"
#include <citro2d.h>
#include <cstdio>
#include <iomanip>
#include <optional>

extern "C" {
#include "hmac.h"
}

#include "firacode_bcfnt.h"

#define WIDTH 400
#define HEIGHT 240
#define SCREEN_SIZE (WIDTH * HEIGHT * 2)

#define BOT_WIDTH 320
#define BOT_HEIGHT 240

#define PADDING 8
#define ROW_WIDTH (WIDTH / 2.f - PADDING - PADDING / 2.f)
#define ROW_HEIGHT 60
#define ROW_LEFT_OFFSET PADDING
#define ROW_RIGHT_OFFSET ROW_WIDTH + PADDING * 2

// REF: https://totp.danhersam.com/
std::string base = base32_decode("JBSWY3DPEHPK3PXP");


u32 clrWhite = C2D_Color32(0xff, 0xff, 0xff, 0xff);
u32 clrBlack = C2D_Color32(0x00, 0x00, 0x00, 0xFF);
u32 clrBlue = C2D_Color32(0x00, 0x00, 0xFF, 0xFF);
u32 clrClear = C2D_Color32(0xFF, 0xFF, 0xFF, 0xFF);
u32 clrRowBackground = C2D_Color32(0xAA, 0xAA, 0xAA, 0xFF);
u32 clrRowBackgroundActive = C2D_Color32(0x88, 0x88, 0x88, 0xFF);


// float lerp(float a, float b, float f)
// {
//     return a * (1.0 - f) + (b * f);
// }

void DrawProgress(const float x, const float y, const float width, const float height, const float percent) {
    C2D_DrawRectangle(x, y, 0, width, height, clrBlack,
                      clrBlack, clrBlack, clrBlack);
    C2D_DrawRectSolid(x, y, 0, percent * width, height,
                      clrBlue);
}


namespace totp {
    list::list(C3D_RenderTarget *left, C3D_RenderTarget *right, C3D_RenderTarget *bottom,
               std::shared_ptr<store> secret_store): left(left), right(right), bottom(bottom),
                                                     secret_store(std::move(secret_store)) {
        code_buf = C2D_TextBufNew(64);
        code_font = C2D_FontLoadFromMem(firacode_bcfnt, firacode_bcfnt_size);
    }

    list::~list() { C2D_TextBufDelete(code_buf); }

    std::optional<scene_type> list::update() {
        std::optional<scene_type> result;
        const u32 kDown = hidKeysDown();
        const u32 kHeld = hidKeysHeld();
        if (kDown & KEY_START) {
            printf("Exit\n");
            return scene_type::EXIT;
        }


        if (kDown & KEY_X) {
            result = scene_type::SCAN;
        }

        if (kDown & KEY_Y) {
            secret_store->save_entries();
            secret_store->load_entries();
        }

        if (kDown & KEY_SELECT) {
            secret_store->remove_entry(selected_item);
            if (selected_item > 0) {
                selected_item--;
            }
        }

        if (kHeld & KEY_L && kHeld & KEY_R) {
            secret_store->reset_entries();
        }

        if (kDown & KEY_DOWN) {
            if (selected_item + 2 < secret_store->get_entries().size()) {
                selected_item += 2;
                const float y = (static_cast<float>(selected_item / 2) * (ROW_HEIGHT + PADDING)) + PADDING;
                if (y + ROW_HEIGHT > HEIGHT + target_scroll_y) {
                    target_scroll_y = y + ROW_HEIGHT + PADDING - HEIGHT;
                }
            }
        }
        if (kDown & KEY_UP) {
            if (selected_item - 2 >= 0) {
                selected_item -= 2;

                const float y = (static_cast<float>(selected_item / 2) * (ROW_HEIGHT + PADDING)) + PADDING;
                if (y < target_scroll_y) {
                    target_scroll_y = y - PADDING;
                }
            }
        }
        if (kDown & KEY_RIGHT) {
            if (selected_item % 2 == 0 && selected_item + 1 < secret_store->get_entries().size()) {
                selected_item += 1;
            }
        }
        if (kDown & KEY_LEFT) {
            if (selected_item % 2 == 1 && selected_item - 1 >= 0) {
                selected_item -= 1;
            }
        }

        scroll_y = std::lerp(target_scroll_y, scroll_y, 0.8f);

        auto iod = osGet3DSliderState() * 3;
        C2D_TargetClear(left, clrWhite);
        C2D_SceneBegin(left);
        render_rows(left, -iod);
        if (iod > 0) {
            C2D_TargetClear(right, clrWhite);
            C2D_SceneBegin(right);
            render_rows(right, iod);
        }
        return result;
    }

    void list::render_rows(C3D_RenderTarget *target, float iod) {
        C2D_TargetClear(target, clrWhite);
        C2D_SceneBegin(target);
        C3D_Mtx mtx;
        C2D_ViewSave(&mtx);
        C2D_ViewTranslate(0, -scroll_y);

        const auto entries = secret_store->get_entries();

        for (unsigned int i = 0; i < entries.size(); i++) {
            const float x = i % 2 == 0 ? ROW_LEFT_OFFSET : ROW_RIGHT_OFFSET;
            const float y = (static_cast<float>(i / 2) * (ROW_HEIGHT + PADDING)) + PADDING;
            const float offset = selected_item == i ? iod : 0;

            C2D_DrawRectSolid(x - offset, y, 0, ROW_WIDTH, ROW_HEIGHT,
                              selected_item == i ? clrRowBackgroundActive : clrRowBackground);
            draw_text(entries[i].get_label(), {x + PADDING / 2 - offset, y + PADDING / 2 - 6}, 1);
            std::stringstream ss;
            ss << std::setw(6) << std::setfill('0') << entries[i].get_code() << " ";
            draw_text(ss.str(), {x + ROW_WIDTH / 2 - offset, y + PADDING / 2 + 20}, 1, C2D_AlignCenter);
        }
        const float percent = static_cast<float>(get_seconds_remaining()) / 30;

        C2D_DrawRectSolid((WIDTH / 2) - (WIDTH * percent) / 2, HEIGHT - 4, 0, (WIDTH * percent), 4,
                          clrBlue);

        C2D_ViewRestore(&mtx);
    }

    void list::draw_text(const std::string &text, const vec2 pos, const float scale, const u32 align) const {
        C2D_TextBufClear(code_buf);
        C2D_Text code_text;
        C2D_TextFontParse(&code_text, code_font, code_buf, text.c_str());
        C2D_TextOptimize(&code_text);

        C2D_DrawText(&code_text, C2D_WithColor | align, pos.x, pos.y, 0, scale, scale, clrBlack);
    }
} // namespace totp
