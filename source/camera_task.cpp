#include "camera_task.h"

#include <cstring>
#include <iostream>

#define EVENT_CANCEL 0
#define EVENT_RECV 1
#define EVENT_BUFFER_ERROR 2

#define EVENT_COUNT 3

namespace totp {
    void camera_task_thread(void *arg) {
        camera_task_data *data = (camera_task_data *) arg;

        Handle events[EVENT_COUNT] = {0};
        events[EVENT_CANCEL] = data->cancel_event;

        Result res = 0;

        u32 bufferSize = data->width * data->height * sizeof(u16);
        u16 *buffer = (u16 *) calloc(1, bufferSize);

        if (R_SUCCEEDED(camInit())) {
            u32 cam = SELECT_OUT1;

            if (
                R_SUCCEEDED(res = CAMU_SetSize(cam, SIZE_CTR_TOP_LCD,CONTEXT_A)) &&
                R_SUCCEEDED(res = CAMU_SetOutputFormat(cam, OUTPUT_RGB_565, CONTEXT_A)) &&
                R_SUCCEEDED(res = CAMU_SetFrameRate(cam, FRAME_RATE_30)) &&
                R_SUCCEEDED(res = CAMU_SetNoiseFilter(cam, true)) &&
                R_SUCCEEDED(res = CAMU_SetAutoExposure(cam, true)) &&
                R_SUCCEEDED(res = CAMU_SetAutoWhiteBalance(cam, true)) &&
                R_SUCCEEDED(res = CAMU_Activate(cam))
            ) {
                u32 transfer_unit = 0;

                if (
                    R_SUCCEEDED(res = CAMU_GetBufferErrorInterruptEvent(&events[EVENT_BUFFER_ERROR], PORT_CAM1)) &&
                    R_SUCCEEDED(res = CAMU_SetTrimming(PORT_CAM1, true)) &&
                    R_SUCCEEDED(res = CAMU_SetTrimmingParamsCenter(PORT_CAM1, data->width, data->height, 400, 240)) &&
                    R_SUCCEEDED(res = CAMU_GetMaxBytes(&transfer_unit, data->width, data->height)) &&
                    R_SUCCEEDED(res = CAMU_SetTransferBytes(PORT_CAM1, transfer_unit, data->width, data->height)) &&
                    R_SUCCEEDED(res = CAMU_ClearBuffer(PORT_CAM1)) &&
                    R_SUCCEEDED(
                        res = CAMU_SetReceiving(&events[EVENT_RECV], buffer, PORT_CAM1, bufferSize, (s16)transfer_unit))
                    &&
                    R_SUCCEEDED(res = CAMU_StartCapture(PORT_CAM1))
                ) {
                    bool cancel_requested = false;
                    while (!cancel_requested && R_SUCCEEDED(res)) {
                        s32 index = 0;
                        if (R_SUCCEEDED(res = svcWaitSynchronizationN(&index, events, EVENT_COUNT, false, U64_MAX))) {
                            switch (index) {
                                case EVENT_CANCEL:
                                    cancel_requested = true;
                                    break;
                                case EVENT_RECV:
                                    svcCloseHandle(events[EVENT_RECV]);
                                    events[EVENT_RECV] = 0;

                                    svcWaitSynchronization(data->mutex, U64_MAX);
                                    memcpy(data->buffer, buffer, bufferSize);
                                    GSPGPU_FlushDataCache(data->buffer, bufferSize);
                                    svcReleaseMutex(data->mutex);

                                    res = CAMU_SetReceiving(&events[EVENT_RECV], buffer, PORT_CAM1, bufferSize,
                                                            (s16) transfer_unit);
                                    break;
                                case EVENT_BUFFER_ERROR:
                                    printf("Buffer error\n");
                                    svcCloseHandle(events[EVENT_RECV]);
                                    events[EVENT_RECV] = 0;

                                    if (
                                        R_SUCCEEDED(res = CAMU_ClearBuffer(PORT_CAM1)) &&
                                        R_SUCCEEDED(
                                            res = CAMU_SetReceiving(&events[EVENT_RECV], buffer, PORT_CAM1,
                                                bufferSize, (s16) transfer_unit))
                                    ) {
                                        res = CAMU_StartCapture(PORT_CAM1);
                                    }
                                    break;
                                default: break;
                            }
                        }
                    }

                    CAMU_StopCapture(PORT_CAM1);

                    bool busy = false;
                    while (R_SUCCEEDED(CAMU_IsBusy(&busy, PORT_CAM1)) && busy) {
                        svcSleepThread(1000000);
                    }

                    CAMU_ClearBuffer(PORT_CAM1);
                }
                CAMU_Activate(SELECT_NONE);
            }
            camExit();
        }

        free(buffer);

        for (int i = 0; i < EVENT_COUNT; i++) {
            if (events[i] != 0) {
                svcCloseHandle(events[i]);
                events[i] = 0;
            }
        }

        svcCloseHandle(data->mutex);

        data->result = res;
        data->finished = true;
    }

    Result camera_task(camera_task_data *data) {
        if (data == nullptr || data->buffer == nullptr || data->width == 0 || data->height == 0) {
            std::cout << "Failed to create camera task" << std::endl;
        }

        data->mutex = 0;
        data->finished = false;
        data->result = 0;
        data->cancel_event = 0;

        Result res = 0;

        if (R_SUCCEEDED(res = svcCreateEvent(&data->cancel_event, RESET_STICKY)) && R_SUCCEEDED(
                res = svcCreateMutex(&data->mutex, false))) {
            if (threadCreate(camera_task_thread, data, 0x10000, 0x1A, 0, true) == NULL) {
                std::cout << "Failed to create camera task thread" << std::endl;
            }
        }

        if (R_FAILED(res)) {
            data->finished = true;

            if (data->cancel_event != 0) {
                svcCloseHandle(data->cancel_event);
                data->cancel_event = 0;
            }

            if (data->mutex != 0) {
                svcCloseHandle(data->mutex);
                data->mutex = 0;
            }
        }

        return res;
    }
}
