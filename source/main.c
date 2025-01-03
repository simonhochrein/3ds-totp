#include "3ds/services/cam.h"
#include "quirc/quirc.h"
#include <3ds.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define WIDTH 400
#define HEIGHT 240
#define SCREEN_SIZE WIDTH *HEIGHT * 2
#define BUF_SIZE SCREEN_SIZE

#define WAIT_TIMEOUT 300000000ULL

void takePicture(u8 *buf) {
  u32 bufSize;
  CAMU_GetMaxBytes(&bufSize, WIDTH, HEIGHT);
  CAMU_SetTransferBytes(PORT_CAM1, bufSize, WIDTH, HEIGHT);
  CAMU_Activate(SELECT_OUT1);

  Handle camReceiveEvent = 0;

  CAMU_ClearBuffer(PORT_CAM1);
  // CAMU_SynchronizeVsyncTiming(SELECT_OUT1, SELECT_OUT2);
  CAMU_StartCapture(PORT_CAM1);

  CAMU_SetReceiving(&camReceiveEvent, buf, PORT_CAM1, SCREEN_SIZE,
                    (s16)bufSize);

  svcWaitSynchronization(camReceiveEvent, WAIT_TIMEOUT);

  CAMU_StopCapture(PORT_CAM1);

  svcCloseHandle(camReceiveEvent);

  CAMU_Activate(SELECT_NONE);
}

void writePictureToFramebufferRGB565(void *fb, void *img, u16 x, u16 y,
                                     u16 width, u16 height) {
  u8 *fb_8 = (u8 *)fb;
  u16 *img_16 = (u16 *)img;
  int i, j, draw_x, draw_y;
  for (j = 0; j < height; j++) {
    for (i = 0; i < width; i++) {
      draw_y = y + height - j;
      draw_x = x + i;
      u32 v = (draw_y + draw_x * height) * 3;
      u16 data = img_16[j * width + i];
      uint8_t b = ((data >> 11) & 0x1F) << 3;
      uint8_t g = ((data >> 5) & 0x3F) << 2;
      uint8_t r = (data & 0x1F) << 3;
      fb_8[v] = r;
      fb_8[v + 1] = g;
      fb_8[v + 2] = b;
    }
  }
}

void scan(u16* buf) {
    struct quirc* ctx= quirc_new();
    quirc_resize(ctx, WIDTH, HEIGHT);
    int w,h;
    uint8_t *qrBuf = quirc_begin(ctx, &w, &h);

    for(int x = 0; x < w; x++) {
        for(int y = 0; y < h; y++) {
            u16 px = buf[y*WIDTH+x];
            qrBuf[y * w + x] = (u8) (((((px >> 11) & 0x1F) << 3) + (((px >> 5) & 0x3F) << 2) + ((px & 0x1F) << 3)) / 3);
        }
    }
    quirc_end(ctx);

    for(int i = 0; i < quirc_count(ctx); i++) {
        struct quirc_code code;
        struct quirc_data data;

        quirc_extract(ctx, i, &code);
        quirc_decode_error_t err = quirc_decode(&code, &data);
        if(err == 0) {
            printf("Found Data: %s\n", data.payload);
        }
    }
}

int main(int argc, char *argv[]) {
  acInit();
  gfxInitDefault();
  consoleInit(GFX_BOTTOM, NULL);
  gfxSetDoubleBuffering(GFX_TOP, true);

  camInit();
  CAMU_SetSize(SELECT_OUT1, SIZE_CTR_TOP_LCD, CONTEXT_A);
  CAMU_SetOutputFormat(SELECT_OUT1, OUTPUT_RGB_565, CONTEXT_A);
  CAMU_SetNoiseFilter(SELECT_OUT1, true);
  CAMU_SetAutoExposure(SELECT_OUT1, true);
  CAMU_SetAutoWhiteBalance(SELECT_OUT1, true);
  CAMU_SetTrimming(PORT_CAM1, false);

  u8 *buf = malloc(BUF_SIZE);

  gfxFlushBuffers();
  gspWaitForVBlank();
  gfxSwapBuffers();

  u32 kDown, kHeld;
  bool held_R = false;

  // Main loop
  while (aptMainLoop()) {
    hidScanInput();
    kDown = hidKeysDown();
    kHeld = hidKeysHeld();

    if (kDown & KEY_START) {
      break;
    }

    if ((kHeld & KEY_R) && !held_R) {
      printf("Capturing new image\n");
      gfxFlushBuffers();
      gspWaitForVBlank();
      gfxSwapBuffers();
      held_R = true;
      takePicture(buf);
      scan((u16*)buf);
    } else if (!(kHeld & KEY_R)) {
      held_R = false;
    }

    writePictureToFramebufferRGB565(
        gfxGetFramebuffer(GFX_TOP, GFX_LEFT, NULL, NULL), buf, 0, 0, WIDTH,
        HEIGHT);

    gfxFlushBuffers();
    gspWaitForVBlank();
    gfxSwapBuffers();
  }

  free(buf);

  gfxExit();
  return 0;
}
