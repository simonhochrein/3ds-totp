#pragma once
#include <3ds.h>
#include <camera_task.h>
#include <optional>
#include <citro3d.h>
#include <citro2d.h>
#include <memory>
extern "C" {
#include <quirc.h>
}
#include "store.h"
#include "scene.h"

namespace totp {
    class scan final : public scene {
    public:
        explicit scan(std::shared_ptr<store> secret_store);
        ~scan() override;
        [[nodiscard]] std::optional<scene_type> update() override;
    private:
        C3D_Tex *tex;
        C2D_Image image{};

        quirc* qr_context;
        bool capturing;
        camera_task_data capture_info;

        std::shared_ptr<store> secret_store;
    };
}
