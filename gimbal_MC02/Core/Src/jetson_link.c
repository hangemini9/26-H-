#include "jetson_link.h"
#include "dm4310.h"
#include "gimbal_app.h"
#include "gimbal_config.h"
#include "ti_link.h"
#include "usb_console.h"
#include <string.h>

#define JETSON_PROTOCOL_VERSION             0x03U
#define JETSON_MAX_PAYLOAD                    64U
#define JETSON_RX_RING_SIZE                  512U
#if GIMBAL_ENABLE_DELAY_TEST
#define JETSON_STATUS_PERIOD_MS \
    GIMBAL_DELAY_TEST_STATUS_PERIOD_MS
#else
#define JETSON_STATUS_PERIOD_MS \
    GIMBAL_NORMAL_STATUS_PERIOD_MS
#endif
#define JETSON_EVENT_QUEUE_SIZE                8U

#define JETSON_MSG_HEARTBEAT                 0x01U
#define JETSON_MSG_VISION_SAMPLE             0x10U
#define JETSON_MSG_GIMBAL_STATUS             0x81U
#define JETSON_MSG_SYSTEM_EVENT              0x82U

#define JETSON_FLAG_CAMERA_READY          (1U << 0)
#define JETSON_FLAG_DETECTOR_READY        (1U << 1)
#define JETSON_FLAG_CALIBRATION_READY     (1U << 2)

#define STATUS_FLAG_JETSON_ONLINE         (1U << 0)
#define STATUS_FLAG_VISION_ACCEPTED       (1U << 1)
#define STATUS_FLAG_MOTOR_ONLINE          (1U << 2)
#define STATUS_FLAG_CHASSIS_ONLINE        (1U << 3)
#define STATUS_FLAG_RECORDING_REQUESTED   (1U << 4)
#define STATUS_FLAG_MOTOR_ENABLED         (1U << 5)
#define STATUS_FLAG_STOP_LATCHED          (1U << 6)

typedef struct
{
    uint32_t run_id;
    uint16_t event_code;
    uint16_t detail;
    uint32_t time_ms;
} JetsonEvent;

static volatile uint8_t s_rx_ring[JETSON_RX_RING_SIZE];
static volatile uint16_t s_rx_head;
static volatile uint16_t s_rx_tail;
static volatile uint32_t s_rx_overflow;
static uint8_t s_parse_state;
static uint8_t s_parse_body[6U + JETSON_MAX_PAYLOAD];
static uint16_t s_parse_index;
static uint16_t s_parse_body_length;
static uint16_t s_parse_crc;
static uint16_t s_tx_sequence;
static uint16_t s_last_rx_sequence;
static uint8_t s_have_rx_sequence;
static uint8_t s_online;
static uint8_t s_pipeline_state;
static uint8_t s_heartbeat_flags;
static uint32_t s_last_heartbeat_ms;
static uint32_t s_last_status_ms;
static uint32_t s_rx_frames;
static uint32_t s_crc_errors;
static uint32_t s_sequence_drops;
static uint32_t s_tx_frames;
static uint32_t s_tx_drops;
static JetsonEvent s_event_queue[JETSON_EVENT_QUEUE_SIZE];
static uint8_t s_event_head;
static uint8_t s_event_tail;

static uint16_t ring_next(uint16_t value)
{
    return (uint16_t)((value + 1U) % JETSON_RX_RING_SIZE);
}

static uint8_t event_next(uint8_t value)
{
    return (uint8_t)((value + 1U) % JETSON_EVENT_QUEUE_SIZE);
}

static uint16_t read_u16(const uint8_t *data)
{
    return (uint16_t)data[0] | ((uint16_t)data[1] << 8);
}

static uint32_t read_u32(const uint8_t *data)
{
    return (uint32_t)data[0] |
           ((uint32_t)data[1] << 8) |
           ((uint32_t)data[2] << 16) |
           ((uint32_t)data[3] << 24);
}

static int32_t read_i32(const uint8_t *data)
{
    return (int32_t)read_u32(data);
}

static void write_u16(uint8_t *data, uint16_t value)
{
    data[0] = (uint8_t)value;
    data[1] = (uint8_t)(value >> 8);
}

static void write_u32(uint8_t *data, uint32_t value)
{
    data[0] = (uint8_t)value;
    data[1] = (uint8_t)(value >> 8);
    data[2] = (uint8_t)(value >> 16);
    data[3] = (uint8_t)(value >> 24);
}

uint16_t jetson_link_crc16(const uint8_t *data, uint16_t length)
{
    uint16_t crc = 0xFFFFU;
    for (uint16_t index = 0U; index < length; index++)
    {
        crc ^= (uint16_t)data[index] << 8;
        for (uint8_t bit = 0U; bit < 8U; bit++)
        {
            crc = (crc & 0x8000U) != 0U ?
                  (uint16_t)((crc << 1) ^ 0x1021U) :
                  (uint16_t)(crc << 1);
        }
    }
    return crc;
}

static uint8_t send_frame(
    uint8_t type, const uint8_t *payload, uint16_t length)
{
    uint8_t frame[10U + JETSON_MAX_PAYLOAD];
    uint16_t sequence = (uint16_t)(s_tx_sequence + 1U);
    uint16_t crc;

    if (length > JETSON_MAX_PAYLOAD)
    {
        return 0U;
    }
    frame[0] = 0xA5U;
    frame[1] = 0x5AU;
    frame[2] = JETSON_PROTOCOL_VERSION;
    frame[3] = type;
    write_u16(&frame[4], length);
    write_u16(&frame[6], sequence);
    if ((payload != NULL) && (length != 0U))
    {
        memcpy(&frame[8], payload, length);
    }
    crc = jetson_link_crc16(&frame[2], (uint16_t)(6U + length));
    write_u16(&frame[8U + length], crc);

    if (usb_console_write_bytes(frame, (uint16_t)(10U + length)) == 0U)
    {
        return 0U;
    }
    s_tx_sequence = sequence;
    s_tx_frames++;
    return 1U;
}

static void process_heartbeat(
    const uint8_t *payload, uint16_t length, uint32_t now_ms)
{
    if (length != 8U)
    {
        return;
    }
    s_pipeline_state = payload[4];
    s_heartbeat_flags = payload[5];
    s_last_heartbeat_ms = now_ms;
    s_online = 1U;
}

static void process_vision(
    const uint8_t *payload, uint16_t length,
    uint16_t sequence, uint32_t now_ms)
{
    VisionSample sample;
    uint32_t active_run = ti_link_run_id();

    if (length != 32U)
    {
        return;
    }
    memset(&sample, 0, sizeof(sample));
    sample.run_id = read_u32(&payload[0]);
    if ((active_run != 0U && sample.run_id != active_run) ||
        (active_run == 0U && sample.run_id != 0U))
    {
        return;
    }
    sample.sequence = sequence;
    sample.capture_time_ms = read_u32(&payload[4]);
    sample.processing_latency_us = read_u32(&payload[8]);
    sample.receive_time_ms = now_ms;
    sample.position_um = read_i32(&payload[12]);
    sample.velocity_um_s = read_i32(&payload[16]);
    sample.confidence_permille = read_u16(&payload[20]);
    sample.flags = read_u16(&payload[22]);
    if ((sample.position_um < -130000L) ||
        (sample.position_um > 130000L) ||
        (sample.confidence_permille > 1000U))
    {
        sample.flags = 0U;
        sample.position_um = 0;
        sample.velocity_um_s = 0;
        sample.confidence_permille = 0U;
    }
    gimbal_app_on_vision_sample(&sample);
}

static void process_frame(uint32_t now_ms)
{
    uint8_t type = s_parse_body[1];
    uint16_t length = read_u16(&s_parse_body[2]);
    uint16_t sequence = read_u16(&s_parse_body[4]);
    const uint8_t *payload = &s_parse_body[6];

    if (s_have_rx_sequence != 0U)
    {
        int16_t delta = (int16_t)(sequence - s_last_rx_sequence);
        if (delta <= 0)
        {
            s_sequence_drops++;
            return;
        }
        if (delta > 1)
        {
            s_sequence_drops += (uint16_t)(delta - 1);
        }
    }
    s_have_rx_sequence = 1U;
    s_last_rx_sequence = sequence;
    s_rx_frames++;
    if (type == JETSON_MSG_HEARTBEAT)
    {
        process_heartbeat(payload, length, now_ms);
    }
    else if (type == JETSON_MSG_VISION_SAMPLE)
    {
        process_vision(payload, length, sequence, now_ms);
    }
}

static void reset_parser(void)
{
    s_parse_state = 0U;
    s_parse_index = 0U;
    s_parse_body_length = 0U;
    s_parse_crc = 0U;
}

static void consume_byte(uint8_t byte, uint32_t now_ms)
{
    if (s_parse_state == 0U)
    {
        if (byte == 0xA5U)
        {
            s_parse_state = 1U;
        }
        return;
    }
    if (s_parse_state == 1U)
    {
        if (byte == 0x5AU)
        {
            s_parse_state = 2U;
            s_parse_index = 0U;
            s_parse_body_length = 0U;
        }
        else if (byte != 0xA5U)
        {
            reset_parser();
        }
        return;
    }
    if (s_parse_state == 2U)
    {
        s_parse_body[s_parse_index++] = byte;
        if (s_parse_index == 4U)
        {
            uint16_t length = read_u16(&s_parse_body[2]);
            if ((s_parse_body[0] != JETSON_PROTOCOL_VERSION) ||
                (length > JETSON_MAX_PAYLOAD))
            {
                reset_parser();
                return;
            }
            s_parse_body_length = (uint16_t)(6U + length);
        }
        if ((s_parse_body_length != 0U) &&
            (s_parse_index >= s_parse_body_length))
        {
            s_parse_state = 3U;
        }
        return;
    }
    if (s_parse_state == 3U)
    {
        s_parse_crc = byte;
        s_parse_state = 4U;
        return;
    }

    s_parse_crc |= (uint16_t)byte << 8;
    if (s_parse_crc ==
        jetson_link_crc16(s_parse_body, s_parse_body_length))
    {
        process_frame(now_ms);
    }
    else
    {
        s_crc_errors++;
    }
    reset_parser();
}

static uint8_t rx_pop(uint8_t *byte)
{
    if (s_rx_tail == s_rx_head)
    {
        return 0U;
    }
    *byte = s_rx_ring[s_rx_tail];
    s_rx_tail = ring_next(s_rx_tail);
    return 1U;
}

static uint8_t send_status(uint32_t now_ms)
{
    const Dm4310Status *motor = dm4310_status();
    uint8_t payload[24];
    uint8_t flags = 0U;
    uint32_t run_id = ti_link_run_id();
    uint8_t question_id = ti_link_question_id();
    uint16_t age = gimbal_app_vision_age_ms(now_ms);

    if (s_online != 0U)
    {
        flags |= STATUS_FLAG_JETSON_ONLINE;
    }
    if ((age <= GIMBAL_VISION_STALE_MS) &&
        ((g_gimbal_debug.vision_flags &
          (VISION_FLAG_BALL_VALID | VISION_FLAG_PIPE_VALID |
           VISION_FLAG_CALIBRATED)) ==
         (VISION_FLAG_BALL_VALID | VISION_FLAG_PIPE_VALID |
          VISION_FLAG_CALIBRATED)))
    {
        flags |= STATUS_FLAG_VISION_ACCEPTED;
    }
    if (motor->online != 0U)
    {
        flags |= STATUS_FLAG_MOTOR_ONLINE;
    }
    if (ti_link_chassis_online() != 0U)
    {
        flags |= STATUS_FLAG_CHASSIS_ONLINE;
    }
    if (question_id != 0U)
    {
        flags |= STATUS_FLAG_RECORDING_REQUESTED;
    }
    if (gimbal_app_motor_enabled() != 0U)
    {
        flags |= STATUS_FLAG_MOTOR_ENABLED;
    }
    if (gimbal_app_stop_latched() != 0U)
    {
        flags |= STATUS_FLAG_STOP_LATCHED;
    }

    write_u32(&payload[0], now_ms);
    write_u32(&payload[4], run_id);
    payload[8] = question_id;
    payload[9] = gimbal_app_system_state();
    write_u16(&payload[10], (uint16_t)g_gimbal_debug.fault);
    write_u32(&payload[12],
              (uint32_t)gimbal_app_target_position_um());
    write_u32(&payload[16],
              (uint32_t)gimbal_app_command_pipe_angle_mdeg());
    write_u16(&payload[20], age);
    payload[22] = motor->error;
    payload[23] = flags;
    return send_frame(
        JETSON_MSG_GIMBAL_STATUS, payload, sizeof(payload));
}

void jetson_link_init(uint32_t now_ms)
{
    static const uint8_t crc_vector[] = {
        0x03U, 0x01U, 0x08U, 0x00U, 0x01U, 0x00U,
        0x78U, 0x56U, 0x34U, 0x12U, 0x02U, 0x07U,
        0x00U, 0x00U
    };
    s_rx_head = 0U;
    s_rx_tail = 0U;
    s_rx_overflow = 0U;
    reset_parser();
    s_tx_sequence = 0U;
    s_last_rx_sequence = 0U;
    s_have_rx_sequence = 0U;
    s_online = 0U;
    s_pipeline_state = 0U;
    s_heartbeat_flags = 0U;
    s_last_heartbeat_ms = now_ms;
    s_last_status_ms = now_ms;
    s_rx_frames = 0U;
    s_crc_errors =
        jetson_link_crc16(crc_vector, sizeof(crc_vector)) == 0xF12FU ?
        0U : 1U;
    s_sequence_drops = 0U;
    s_tx_frames = 0U;
    s_tx_drops = 0U;
    s_event_head = 0U;
    s_event_tail = 0U;
}

void jetson_link_rx_isr(const uint8_t *data, uint32_t length)
{
    if (data == NULL)
    {
        return;
    }
    for (uint32_t index = 0U; index < length; index++)
    {
        uint16_t next = ring_next(s_rx_head);
        if (next == s_rx_tail)
        {
            s_rx_overflow++;
            continue;
        }
        s_rx_ring[s_rx_head] = data[index];
        s_rx_head = next;
    }
}

void jetson_link_send_event(
    uint32_t run_id, uint16_t event_code, uint16_t detail,
    uint32_t now_ms)
{
    uint8_t next = event_next(s_event_head);
    if (next == s_event_tail)
    {
        s_tx_drops++;
        return;
    }
    s_event_queue[s_event_head].run_id = run_id;
    s_event_queue[s_event_head].event_code = event_code;
    s_event_queue[s_event_head].detail = detail;
    s_event_queue[s_event_head].time_ms = now_ms;
    s_event_head = next;
}

void jetson_link_poll(uint32_t now_ms)
{
    uint8_t byte;
    while (rx_pop(&byte) != 0U)
    {
        consume_byte(byte, now_ms);
    }
    if (s_rx_overflow != 0U)
    {
        s_rx_tail = s_rx_head;
        s_rx_overflow = 0U;
        reset_parser();
    }
    if ((s_online != 0U) &&
        ((uint32_t)(now_ms - s_last_heartbeat_ms) >
         GIMBAL_JETSON_HEARTBEAT_LOST_MS))
    {
        s_online = 0U;
        s_have_rx_sequence = 0U;
        if (ti_link_run_id() != 0U)
        {
            gimbal_app_stop_question(
                ti_link_run_id(), 1U, now_ms);
        }
    }

    if (s_event_tail != s_event_head)
    {
        uint8_t payload[12];
        const JetsonEvent *event = &s_event_queue[s_event_tail];
        write_u32(&payload[0], event->run_id);
        write_u16(&payload[4], event->event_code);
        write_u16(&payload[6], event->detail);
        write_u32(&payload[8], event->time_ms);
        if (send_frame(
                JETSON_MSG_SYSTEM_EVENT, payload, sizeof(payload)) != 0U)
        {
            s_event_tail = event_next(s_event_tail);
        }
    }
    else if ((uint32_t)(now_ms - s_last_status_ms) >=
             JETSON_STATUS_PERIOD_MS)
    {
        if (send_status(now_ms) != 0U)
        {
            s_last_status_ms = now_ms;
        }
        else
        {
            s_tx_drops++;
            s_last_status_ms = now_ms;
        }
    }

    g_gimbal_debug.jetson_online = s_online;
    g_gimbal_debug.jetson_pipeline_state = s_pipeline_state;
    g_gimbal_debug.jetson_flags = s_heartbeat_flags;
    g_gimbal_debug.jetson_heartbeat_age_ms =
        s_online != 0U ?
        (uint32_t)(now_ms - s_last_heartbeat_ms) : 0xFFFFFFFFUL;
    g_gimbal_debug.jetson_rx_frames = s_rx_frames;
    g_gimbal_debug.jetson_crc_errors = s_crc_errors;
    g_gimbal_debug.jetson_sequence_drops = s_sequence_drops;
    g_gimbal_debug.jetson_tx_frames = s_tx_frames;
    g_gimbal_debug.jetson_tx_drops = s_tx_drops;
}

uint8_t jetson_link_online(void)
{
    return s_online;
}

uint8_t jetson_link_ready(void)
{
    uint8_t required = JETSON_FLAG_CAMERA_READY |
                       JETSON_FLAG_DETECTOR_READY |
                       JETSON_FLAG_CALIBRATION_READY;
    return ((s_online != 0U) &&
            (s_pipeline_state == 2U) &&
            ((s_heartbeat_flags & required) == required)) ? 1U : 0U;
}

uint32_t jetson_link_last_heartbeat_ms(void)
{
    return s_last_heartbeat_ms;
}
