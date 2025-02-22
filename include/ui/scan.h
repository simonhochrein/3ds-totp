#pragma once
#include <3ds.h>
#include <optional>
#include <citro3d.h>
#include <citro2d.h>
#include "scene.h"

namespace totp {
    class scan final : public scene {
    public:
        scan();
        ~scan() override;
        [[nodiscard]] std::optional<scene_type> update() override;
    private:
        u16* camera_buffer;
        Handle mutex;
        Handle cancel;
        bool capturing;
        volatile bool finished;
        C3D_Tex *tex;
        C2D_Image image{};
        static void cam_thread(scan* app);
        bool do_scan();
        void cleanup();
    };
}
