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
                scene = std::make_unique<totp::scan>(store);
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
