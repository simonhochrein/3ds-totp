#include "ui/scan.h"
#include "3ds/services/cam.h"
#include "3ds/svc.h"
#include "3ds/types.h"
#include "scene.h"
#include <cstddef>
#include <cstdlib>
#include <optional>

#include "quirc.h"

#define WIDTH 400
#define HEIGHT 240

namespace totp {
scan::scan() {
  capturing = false;
  finished = false;
  camera_buffer = new u16[400 * 240];
  tex = new C3D_Tex;
  static const Tex3DS_SubTexture subtex = {512, 256, 0, 1, 1, 0};
  image = C2D_Image{.tex = tex, .subtex = &subtex};
  C3D_TexInit(tex, 512, 256, GPU_RGB565);
  C3D_TexSetFilter(tex, GPU_LINEAR, GPU_LINEAR);
}
scan::~scan() {
  delete[] camera_buffer;
  delete tex;
  C3D_TexDelete(tex);
}

std::optional<scene_type> scan::update() {
  std::optional<scene_type> result;

  if (hidKeysDown() & KEY_START) {
    cleanup();
    return scene_type::LIST;
  }

  if (!capturing) {
    mutex = 0;
    svcCreateEvent(&cancel, RESET_STICKY);
    svcCreateMutex(&mutex, false);
    if (threadCreate(reinterpret_cast<ThreadFunc>(cam_thread), this, 0x10000,
                     0x1A, 0, true) != NULL) {
      capturing = true;
    } else {
      cleanup();
      return scene_type::EXIT;
    }
  }
  svcWaitSynchronization(mutex, U64_MAX);
  for (u32 x = 0; x < 400; x++) {
    for (u32 y = 0; y < 240; y++) {
      u32 dstPos = ((((y >> 3) * (512 >> 3) + (x >> 3)) << 6) +
                    ((x & 1) | ((y & 1) << 1) | ((x & 2) << 1) |
                     ((y & 2) << 2) | ((x & 4) << 2) | ((y & 4) << 3))) *
                   2;
      u32 srcPos = (y * 400 + x) * 2;
      memcpy(&((u8 *)image.tex->data)[dstPos], &((u8 *)camera_buffer)[srcPos],
             2);
    }
  }
  svcReleaseMutex(mutex);

  C2D_DrawImageAt(image, 0, 0, 0.5f, NULL, 1, 1);

  return result;
}

void scan::cam_thread(scan *app) {
  Handle events[3] = {0};
  events[0] = app->cancel;
  u32 transferUnit;

  u16 *buffer = new u16[400 * 240];
  camInit();
  CAMU_SetSize(SELECT_OUT1, SIZE_CTR_TOP_LCD, CONTEXT_A);
  CAMU_SetOutputFormat(SELECT_OUT1, OUTPUT_RGB_565, CONTEXT_A);
  CAMU_SetFrameRate(SELECT_OUT1, FRAME_RATE_30);
  CAMU_SetNoiseFilter(SELECT_OUT1, true);
  CAMU_SetAutoExposure(SELECT_OUT1, true);
  CAMU_SetAutoWhiteBalance(SELECT_OUT1, true);
  CAMU_Activate(SELECT_OUT1);
  CAMU_GetBufferErrorInterruptEvent(&events[2], PORT_CAM1);
  CAMU_SetTrimming(PORT_CAM1, false);
  CAMU_GetMaxBytes(&transferUnit, 400, 240);
  CAMU_SetTransferBytes(PORT_CAM1, transferUnit, 400, 240);
  CAMU_ClearBuffer(PORT_CAM1);
  CAMU_SetReceiving(&events[1], buffer, PORT_CAM1, 400 * 240,
                    (s16)transferUnit);
  CAMU_StartCapture(PORT_CAM1);
  bool cancel = false;
  while (!cancel) {
    s32 index = 0;
    svcWaitSynchronizationN(&index, events, 3, false, U64_MAX);
    switch (index) {
    case 0:
      cancel = true;
      break;
    case 1:
      svcCloseHandle(events[1]);
      events[1] = 0;
      svcWaitSynchronization(app->mutex, U64_MAX);
      memcpy(app->camera_buffer, buffer, 400 * 240 * sizeof(u16));
      app->do_scan();
      GSPGPU_FlushDataCache(app->camera_buffer, 400 * 240 * sizeof(u16));
      svcReleaseMutex(app->mutex);
      CAMU_SetReceiving(&events[1], buffer, PORT_CAM1, 400 * 240 * sizeof(u16),
                        transferUnit);
      break;
    case 2:
      svcCloseHandle(events[1]);
      events[1] = 0;
      CAMU_ClearBuffer(PORT_CAM1);
      CAMU_SetReceiving(&events[1], buffer, PORT_CAM1, 400 * 240 * sizeof(u16),
                        transferUnit);
      CAMU_StartCapture(PORT_CAM1);
      break;
    default:
      break;
    }
  }
  CAMU_StopCapture(PORT_CAM1);

  bool busy = false;
  while (R_SUCCEEDED(CAMU_IsBusy(&busy, PORT_CAM1)) && busy) {
    svcSleepThread(1000000);
  }

  CAMU_ClearBuffer(PORT_CAM1);
  CAMU_Activate(SELECT_NONE);
  camExit();
  delete[] buffer;
  for (int i = 0; i < 3; i++) {
    if (events[i] != 0) {
      svcCloseHandle(events[i]);
      events[i] = 0;
    }
  }
  svcCloseHandle(app->mutex);
  app->finished = true;
}

void scan::cleanup() {
  svcSignalEvent(cancel);
  while (!finished) {
    svcSleepThread(1000000);
  }
  capturing = false;
}

bool scan::do_scan() {
    struct quirc *ctx = quirc_new();
    quirc_resize(ctx, WIDTH, HEIGHT);
    int w, h;
    uint8_t *qrBuf = quirc_begin(ctx, &w, &h);

    for (int x = 0; x < w; x++) {
      for (int y = 0; y < h; y++) {
        u16 px = camera_buffer[y * WIDTH + x];
        qrBuf[y * w + x] = (u8)(((((px >> 11) & 0x1F) << 3) +
                                 (((px >> 5) & 0x3F) << 2) + ((px & 0x1F) << 3)) /
                                3);
      }
    }
    quirc_end(ctx);

    for (int i = 0; i < quirc_count(ctx); i++) {
      struct quirc_code code;
      struct quirc_data data;

      quirc_extract(ctx, i, &code);
      quirc_decode_error_t err = quirc_decode(&code, &data);
      if (err == 0) {
        printf("Found Data: %s\n", data.payload);
        // strcpy((char *)data.payload, out_buf);
        return true;
      }
    }

    quirc_destroy(ctx);
    return false;
}
} // namespace totp
