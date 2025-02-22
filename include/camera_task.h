#pragma once
#include <3ds.h>

namespace totp {
    struct camera_task_data {
        u16* buffer;
        s16 width;
        s16 height;
        Handle mutex;

        volatile bool finished;
        Result result;
        Handle cancel_event;
    };

    static void camera_task_thread(void *arg);

    Result camera_task(camera_task_data* data);
}
