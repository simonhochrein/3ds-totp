#pragma once
#include <3ds.h>
#include <optional>
#include <citro3d.h>
#include <citro2d.h>
#include <memory>
#include "store.h"
#include "scene.h"

namespace totp {
    class scan final : public scene {
    public:
        explicit scan(std::shared_ptr<store> secret_store);
        ~scan() override;
        [[nodiscard]] std::optional<scene_type> update() override;
    private:
        u16* camera_buffer;
        Handle mutex;
        Handle cancel;
        bool capturing;
        volatile bool should_go_back = false;
        volatile bool finished;
        C3D_Tex *tex;
        C2D_Image image{};
        static void cam_thread(scan* app);
        bool do_scan();
        void cleanup();
        std::shared_ptr<store> secret_store;
    };
}
