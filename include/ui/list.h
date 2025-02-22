#pragma once
#include <3ds.h>
#include <citro2d.h>
#include <memory>
#include <store.h>
#include "scene.h"
#include "vec2.h"

namespace totp {
    class list : public scene {
    public:
        list(C3D_RenderTarget *left, C3D_RenderTarget *right, C3D_RenderTarget *bottom,
             std::shared_ptr<store> secret_store);

        ~list() override;

        [[nodiscard]] std::optional<scene_type> update() final;

    private:
        C2D_TextBuf code_buf;
        C2D_Font code_font;
        uint8_t key[64] = {};
        size_t key_len;

        float target_scroll_y = 0;
        float scroll_y = 0;
        int selected_item = 0;
        C3D_RenderTarget *left;
        C3D_RenderTarget *bottom;
        C3D_RenderTarget *right;


        std::shared_ptr<store> secret_store;

        void render_rows(C3D_RenderTarget *target, float iod);

        void draw_text(const std::string &text, vec2 pos, float scale, u32 align = C2D_AlignLeft) const;
    };
} // namespace totp
