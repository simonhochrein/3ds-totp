#include "ui/scan.h"
#include "3ds/services/cam.h"
#include "3ds/svc.h"
#include "3ds/types.h"
#include "scene.h"
#include <optional>
#include <string>
#include <utility>
#include <urlparser.h>

#include "uri.hpp"
#include "quirc.h"

#define WIDTH 400
#define HEIGHT 240

#define QR_IMAGE_WIDTH WIDTH
#define QR_IMAGE_HEIGHT HEIGHT

void load_texture(C3D_Tex *tex, void *data, u32 size, u32 width, u32 height) {
    C3D_TexSetFilter(tex, GPU_NEAREST, GPU_NEAREST);

    u32 pixel_size = size / width / height;
    memset(tex->data, 0, tex->size);
    for (u32 x = 0; x < width; x++) {
        for (u32 y = 0; y < height; y++) {
            u32 dstPos = ((((y >> 3) * (512 >> 3) + (x >> 3)) << 6) + (
                              (x & 1) | ((y & 1) << 1) | ((x & 2) << 1) | ((y & 2) << 2) | ((x & 4) << 2) | (
                                  (y & 4) << 3))) * pixel_size;
            u32 srcPos = (y * width + x) * pixel_size;

            memcpy(&((u8 *) tex->data)[dstPos], &((u8 *) data)[srcPos], pixel_size);
        }
    }

    C3D_TexFlush(tex);
}

namespace totp {
    scan::scan(std::shared_ptr<store> secret_store): secret_store(std::move(secret_store)) {
        tex = new C3D_Tex;
        static const Tex3DS_SubTexture subtex = {512, 256, 0, 1, 1, 0};
        image = C2D_Image{.tex = tex, .subtex = &subtex};
        C3D_TexInit(tex, 512, 256, GPU_RGB565);

        capturing = false;
        capture_info.width = QR_IMAGE_WIDTH;
        capture_info.height = QR_IMAGE_HEIGHT;
        capture_info.finished = true;

        qr_context = quirc_new();
        quirc_resize(qr_context, QR_IMAGE_WIDTH, QR_IMAGE_HEIGHT);
        capture_info.buffer = static_cast<u16 *>(calloc(1, QR_IMAGE_WIDTH * QR_IMAGE_HEIGHT * sizeof(u16)));
    }

    scan::~scan() {
        // delete[] camera_buffer;

        if (!capture_info.finished) {
            svcSignalEvent(capture_info.cancel_event);
            while (!capture_info.finished) {
                svcSleepThread(1000000);
            }
        }

        capturing = false;

        if (capture_info.buffer != nullptr) {
            memset(capture_info.buffer, 0, QR_IMAGE_WIDTH * QR_IMAGE_HEIGHT * sizeof(u16));
        }

        if (capture_info.buffer != nullptr) {
            free(capture_info.buffer);
            capture_info.buffer = nullptr;
        }

        C3D_TexDelete(tex);
        delete tex;

        if (qr_context != nullptr) {
            quirc_destroy(qr_context);
            qr_context = nullptr;
        }
    }

    std::optional<scene_type> scan::update() {
        std::optional<scene_type> result;

        if (hidKeysDown() & KEY_B) {
            result = scene_type::LIST;
        }

        if (!capturing) {
            Result cap_res = camera_task(&capture_info);
            if (R_FAILED(cap_res)) {
                return scene_type::LIST;
            }
            capturing = true;
        }

        if (capture_info.finished) {
            if (R_FAILED(capture_info.result)) {
                printf("Failed to capture %d\n", capture_info.result);
            }

            return scene_type::LIST;
        }

        svcWaitSynchronization(capture_info.mutex, U64_MAX);
        load_texture(image.tex, capture_info.buffer, QR_IMAGE_WIDTH * QR_IMAGE_HEIGHT * sizeof(u16), QR_IMAGE_WIDTH,
                     QR_IMAGE_HEIGHT);
        svcReleaseMutex(capture_info.mutex);

        C2D_DrawImageAt(image, 0, 0, 0.5f, nullptr, 1, 1);


        int w = 0;
        int h = 0;
        uint8_t* qrBuf = quirc_begin(qr_context, &w, &h);

        svcWaitSynchronization(capture_info.mutex, U64_MAX);

        for(int x = 0; x < w; x++) {
            for(int y = 0; y < h; y++) {
                u16 px = capture_info.buffer[y * QR_IMAGE_WIDTH + x];
                qrBuf[y * w + x] = (u8) (((((px >> 11) & 0x1F) << 3) + (((px >> 5) & 0x3F) << 2) + ((px & 0x1F) << 3)) / 3);
            }
        }

        svcReleaseMutex(capture_info.mutex);

        quirc_end(qr_context);

        int qrCount = quirc_count(qr_context);
        for(int i = 0; i < qrCount; i++) {
            quirc_code qrCode;
            quirc_extract(qr_context, i, &qrCode);

            quirc_data qrData;

            if(const quirc_decode_error_t err = quirc_decode(&qrCode, &qrData); err == 0) {
                const std::string_view output(reinterpret_cast<const char *>(qrData.payload), qrData.payload_len);
                if (urlparser url(output); url.is_valid()) {
                    secret_store->add_entry(url.get_entry());
                }
                return scene_type::LIST;
            }
        }
        return result;
    }
}
