#include <3ds.h>
#include <citro2d.h>
#include <cstdio>
#include <memory>
#include <optional>
#include <iostream>
#include "otp_entry.h"
#include "scene.h"
#include "store.h"
#include "ui/list.h"
#include "b32decode.h"
#include "quirc.h"
#include "ui/scan.h"
#include <malloc.h>

#define WIDTH 400
#define HEIGHT 240
#define SCREEN_SIZE (WIDTH *HEIGHT * 2)
#define BUF_SIZE SCREEN_SIZE

#define WAIT_TIMEOUT 300000000ULL

#define TIMEZONE_OFFSET (6 * 3600)

#define DISPLAY_TRANSFER_FLAGS                                                 \
  (GX_TRANSFER_FLIP_VERT(0) | GX_TRANSFER_OUT_TILED(0) |                       \
   GX_TRANSFER_RAW_COPY(0) | GX_TRANSFER_IN_FORMAT(GX_TRANSFER_FMT_RGBA8) |    \
   GX_TRANSFER_OUT_FORMAT(GX_TRANSFER_FMT_RGB8) |                              \
   GX_TRANSFER_SCALING(GX_TRANSFER_SCALE_NO))
//
// bool scan(const u16 *buf, char *out_buf) {
//   struct quirc *ctx = quirc_new();
//   quirc_resize(ctx, WIDTH, HEIGHT);
//   int w, h;
//   uint8_t *qrBuf = quirc_begin(ctx, &w, &h);
//
//   for (int x = 0; x < w; x++) {
//     for (int y = 0; y < h; y++) {
//       u16 px = buf[y * WIDTH + x];
//       qrBuf[y * w + x] = (u8)(((((px >> 11) & 0x1F) << 3) +
//                                (((px >> 5) & 0x3F) << 2) + ((px & 0x1F) << 3)) /
//                               3);
//     }
//   }
//   quirc_end(ctx);
//
//   for (int i = 0; i < quirc_count(ctx); i++) {
//     quirc_code code{};
//     quirc_data data{};
//
//     quirc_extract(ctx, i, &code);
//     if (const quirc_decode_error_t err = quirc_decode(&code, &data); err == 0) {
//       printf("Found Data: %p\n", data.payload);
//       strcpy(reinterpret_cast<char *>(data.payload), out_buf);
//       return true;
//     }
//   }
//
//   quirc_destroy(ctx);
//   return false;
// }
//
// bool scanner(char *out_buf) {
//
//   C3D_Init(C3D_DEFAULT_CMDBUF_SIZE);
//   C2D_Init(C2D_DEFAULT_MAX_OBJECTS);
//   C2D_Prepare();
//
//   C3D_RenderTarget *top = C2D_CreateScreenTarget(GFX_TOP, GFX_LEFT);
//
//   camInit();
//   CAMU_SetSize(SELECT_OUT1, SIZE_CTR_TOP_LCD, CONTEXT_A);
//   CAMU_SetOutputFormat(SELECT_OUT1, OUTPUT_RGB_565, CONTEXT_A);
//   CAMU_SetFrameRate(SELECT_OUT1, FRAME_RATE_30);
//   CAMU_SetNoiseFilter(SELECT_OUT1, true);
//   CAMU_SetAutoExposure(SELECT_OUT1, true);
//   CAMU_SetAutoWhiteBalance(SELECT_OUT1, true);
//   CAMU_SetTrimming(PORT_CAM1, false);
//
//   u8 *buf = static_cast<u8 *>(linearAlloc(SCREEN_SIZE));
//   u32 bufSize;
//   CAMU_GetMaxBytes(&bufSize, WIDTH, HEIGHT);
//   CAMU_SetTransferBytes(PORT_CAM1, bufSize, WIDTH, HEIGHT);
//   CAMU_Activate(SELECT_OUT1);
//   Handle cameraReceiveEvents[2] = {0};
//   CAMU_GetBufferErrorInterruptEvent(&cameraReceiveEvents[0], PORT_CAM1);
//   CAMU_ClearBuffer(PORT_CAM1);
//   CAMU_StartCapture(PORT_CAM1);
//
//   bool captureInterrupted = false;
//
//   u32 clrColor = C2D_Color32(0, 0, 0, 0xFF);
//
//   C3D_Tex tex;
//   Tex3DS_SubTexture subt3x = {512, 256, 0.f, 1.f, 1.f, 0.f};
//   C2D_Image image = {&tex, &subt3x};
//   C3D_TexInit(&tex, 512, 256, GPU_RGB565);
//   C3D_TexSetFilter(image.tex, GPU_LINEAR, GPU_LINEAR);
//
//   while (aptMainLoop()) {
//     hidScanInput();
//     if (const u32 kDown = hidKeysDown(); kDown & KEY_START) {
//       linearFree(buf);
//       return false;
//     }
//
//     C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
//     C2D_TargetClear(top, clrColor);
//     C2D_SceneBegin(top);
//
//     if (cameraReceiveEvents[1] == 0) {
//       CAMU_SetReceiving(&cameraReceiveEvents[1], buf, PORT_CAM1, SCREEN_SIZE,
//                         bufSize);
//     }
//
//     if (captureInterrupted) {
//       CAMU_StartCapture(PORT_CAM1);
//       captureInterrupted = false;
//     }
//
//     s32 index;
//     svcWaitSynchronizationN(&index, cameraReceiveEvents, 2, false,
//                             1000000000ULL);
//
//     switch (index) {
//     case 0: {
//       printf("interrupt\n");
//       svcCloseHandle(cameraReceiveEvents[1]);
//       cameraReceiveEvents[1] = 0;
//       captureInterrupted = true;
//       continue;
//     }
//     case 1: {
//       svcCloseHandle(cameraReceiveEvents[1]);
//       cameraReceiveEvents[1] = 0;
//       break;
//     }
//       default: break;
//     }
//     GSPGPU_FlushDataCache(buf, 400 * 240 * sizeof(u16));
//     for (u32 x = 0; x < 400; x++) {
//       for (u32 y = 0; y < 240; y++) {
//         const u32 dstPos = ((((y >> 3) * (512 >> 3) + (x >> 3)) << 6) +
//                       ((x & 1) | ((y & 1) << 1) | ((x & 2) << 1) |
//                        ((y & 2) << 2) | ((x & 4) << 2) | ((y & 4) << 3))) *
//                      2;
//         const u32 srcPos = (y * 400 + x) * 2;
//         memcpy(&static_cast<u8 *>(image.tex->data)[dstPos], &buf[srcPos], 2);
//       }
//     }
//
//     C2D_DrawImage(image, nullptr, nullptr);
//
//     // writePictureToFramebufferRGB565(gfxGetFramebuffer(GFX_TOP, GFX_LEFT,
//     // NULL, NULL), buf, 0, 0, WIDTH, HEIGHT);
//     if (scan(reinterpret_cast<u16 *>(buf), out_buf)) {
//       break;
//     }
//
//     C3D_FrameEnd(0);
//   }
//
//   C3D_RenderTargetDelete(top);
//   C2D_Fini();
//   C3D_Fini();
//
//   linearFree(buf);
//
//   return true;
// }

#define SOC_ALIGN       0x1000
#define SOC_BUFFERSIZE  0x100000

class Logging {
public:
  Logging() {
    SOC_buffer = static_cast<u32 *>(memalign(SOC_ALIGN, SOC_BUFFERSIZE));
    if (SOC_buffer == nullptr) {
      exit(-1);
    }

    if (int ret = socInit(SOC_buffer, SOC_BUFFERSIZE); ret != 0) {
      exit(-1);
    }

    link3dsStdio();
  }
  ~Logging() {
    socExit();
  }
private:
  u32* SOC_buffer = nullptr;
};


int main(int argc, char *argv[]) {
  acInit();
  gfxInitDefault();
  gfxSet3D(true);

  Logging logging;

  printf("Hello World!\n");
  // consoleInit(GFX_BOTTOM, nullptr);

  gfxSetDoubleBuffering(GFX_TOP, true);
  gfxSetDoubleBuffering(GFX_BOTTOM, false);


  C3D_Init(C3D_DEFAULT_CMDBUF_SIZE);
  C2D_Init(C2D_DEFAULT_MAX_OBJECTS);
  C2D_Prepare();

  cfguInit();

  auto store = std::make_shared<totp::store>();

  C3D_RenderTarget *top = C2D_CreateScreenTarget(GFX_TOP, GFX_LEFT);
  C3D_RenderTarget *right = C2D_CreateScreenTarget(GFX_TOP, GFX_RIGHT);
  C3D_RenderTarget *bottom = C2D_CreateScreenTarget(GFX_BOTTOM, GFX_LEFT);


  std::unique_ptr<totp::scene> scene(new totp::list(top, right, bottom, store));
  std::optional<totp::scene_type> next_scene;

  while (aptMainLoop()) {
    C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
    C2D_TargetClear(top, C2D_Color32(0xff, 0xff, 0xff, 0xff));
    C2D_SceneBegin(top);
    hidScanInput();
    if (scene) {
      next_scene = scene->update();
    }
    C3D_FrameEnd(0);

    if(next_scene) {
        switch(*next_scene) {
            case totp::scene_type::LIST:
                scene = std::make_unique<totp::list>(top, right, bottom, store);
                break;
            case totp::scene_type::SCAN:
                scene = std::make_unique<totp::scan>();
                break;
            case totp::scene_type::SETTINGS:
                break;
            case totp::scene_type::EXIT:
                goto exit;
        }
    }
  }
  exit:

  cfguExit();
  C3D_RenderTargetDelete(top);
  C2D_Fini();
  C3D_Fini();

  gfxExit();

  return 0;
}
