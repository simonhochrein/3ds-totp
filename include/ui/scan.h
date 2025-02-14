#pragma once
#include <3ds.h>
#include <optional>
#include <citro3d.h>
#include <citro2d.h>
#include "scene.h"

namespace totp {
    class scan: public scene {
    public:
        scan();
        [[nodiscard]] std::optional<scene_type> update();
    private:
        u16* camera_buffer;
        Handle mutex;
        Handle cancel;
        bool capturing;
        volatile bool finished;
        C3D_Tex *tex;
        C2D_Image image;
    private:
        static void cam_thread(scan* app);
        bool do_scan();
        void cleanup();
    protected:
        ~scan();
    };
}
