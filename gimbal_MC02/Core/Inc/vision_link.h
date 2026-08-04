#ifndef VISION_LINK_H
#define VISION_LINK_H

#include <stdint.h>

/* Application-level sample after frozen Jetson protocol V3 is decoded. */
typedef struct
{
    uint32_t run_id;
    uint32_t sequence;
    uint32_t capture_time_ms;
    uint32_t processing_latency_us;
    uint32_t receive_time_ms;
    int32_t position_um;
    int32_t velocity_um_s;
    uint16_t confidence_permille;
    uint16_t flags;
} VisionSample;

#define VISION_FLAG_BALL_VALID       (1U << 0)
#define VISION_FLAG_PIPE_VALID       (1U << 1)
#define VISION_FLAG_CALIBRATED       (1U << 2)
#define VISION_FLAG_VELOCITY_VALID   (1U << 3)
#define VISION_FLAG_OCCLUDED         (1U << 4)
#define VISION_FLAG_FRAME_DROPPED    (1U << 5)
#define VISION_FLAG_OUT_OF_RANGE     (1U << 6)

#endif
