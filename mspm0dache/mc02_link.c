#include "mc02_link.h"

#include <limits.h>

#include "chassis.h"
#include "chassis_config.h"
#include "line_sensor.h"
#include "motor.h"
#include "platform.h"
#include "question_timer.h"
#include "test_mode.h"
#include "ti_msp_dl_config.h"

#define MC02_PROTOCOL_VERSION             0x02U
#define MC02_MAX_PAYLOAD                    64U
#define MC02_RX_BUFFER_SIZE                256U
#define MC02_TX_BUFFER_SIZE                512U
#define MC02_STATUS_PERIOD_MS               50U
#define MC02_HEARTBEAT_TIMEOUT_MS          500U
#define MC02_STOPPING_HOLD_MS               60U
#define MC02_Q1_OPTIONS                     0x05UL
#define MC02_Q1_DEFAULT_TIMEOUT_MS        40000UL
#define MC02_Q2_OPTIONS                     0x00UL
#define MC02_Q2_DEFAULT_TIMEOUT_MS        30000UL
#define MC02_Q3_OPTIONS                     0x08UL
#define MC02_Q3_DEFAULT_TIMEOUT_MS         8000UL
#define MC02_Q45_OPTIONS                    0x05UL
#define MC02_Q45_DEFAULT_TIMEOUT_MS       30000UL

#define MC02_MSG_HEARTBEAT                 0x20U
#define MC02_MSG_COMMAND                   0x21U
#define MC02_MSG_STATUS                    0xA0U
#define MC02_MSG_EVENT                     0xA1U
#define MC02_MSG_BUTTON                    0xA2U

#define MC02_ACTION_SAFE_STOP                 0U
#define MC02_ACTION_PREPARE                   1U
#define MC02_ACTION_START                     2U
#define MC02_ACTION_CANCEL                    3U
#define MC02_ACTION_CLEAR_FAULT               4U

#define MC02_STATE_BOOT                       0U
#define MC02_STATE_IDLE_DISARMED              1U
#define MC02_STATE_READY                      2U
#define MC02_STATE_RUNNING                    3U
#define MC02_STATE_STOPPING                   4U
#define MC02_STATE_ROUTE_COMPLETE             5U
#define MC02_STATE_FAULT                      6U
#define MC02_STATE_EMERGENCY_STOP_LATCHED     7U

#define MC02_FAULT_NONE                       0U
#define MC02_FAULT_SUPERVISOR_LOST            1U
#define MC02_FAULT_LINE_SENSOR                2U
#define MC02_FAULT_ENCODER                    3U
#define MC02_FAULT_OUTPUT                     4U
#define MC02_FAULT_BAD_PARAMETER              5U
#define MC02_FAULT_ROUTE_TIMEOUT              6U

#define MC02_EVENT_READY                      1U
#define MC02_EVENT_STARTED                    2U
#define MC02_EVENT_A_MARKER_DETECTED           3U
#define MC02_EVENT_ROUTE_COMPLETE             4U
#define MC02_EVENT_STOPPED                    5U
#define MC02_EVENT_FAULT                      6U
#define MC02_EVENT_B_MARKER_PASSED            7U
#define MC02_EVENT_EMERGENCY_STOPPED          8U

#define MC02_FLAG_LINE_SENSOR_OK          (1U << 0)
#define MC02_FLAG_LEFT_ENCODER_OK         (1U << 1)
#define MC02_FLAG_RIGHT_ENCODER_OK        (1U << 2)
#define MC02_FLAG_MOTOR_OUTPUT_ENABLED    (1U << 3)
#define MC02_FLAG_A_MARKER_SEEN           (1U << 4)
#define MC02_FLAG_SUPERVISOR_ONLINE       (1U << 5)
#define MC02_FLAG_STOP_LATCHED            (1U << 6)
#define MC02_FLAG_EMERGENCY_STOP_LATCHED  (1U << 7)
#define MC02_FLAG_B_MARKER_SEEN           (1U << 8)
#define MC02_FLAG_BUTTON_PANEL_OK         (1U << 9)
#define MC02_FLAG_COMMON_ACCEL_ACTIVE     (1U << 10)
#define MC02_FLAG_COMMON_DECEL_ACTIVE     (1U << 11)

static volatile uint8_t s_rx_buffer[MC02_RX_BUFFER_SIZE];
static volatile uint16_t s_rx_head;
static volatile uint16_t s_rx_tail;
static volatile uint8_t s_rx_overflow;

static volatile uint8_t s_tx_buffer[MC02_TX_BUFFER_SIZE];
static volatile uint16_t s_tx_head;
static volatile uint16_t s_tx_tail;

static uint8_t s_parse_state;
static uint8_t s_parse_body[6U + MC02_MAX_PAYLOAD];
static uint16_t s_parse_index;
static uint16_t s_parse_body_length;
static uint16_t s_parse_crc;

static uint16_t s_tx_sequence;
static uint16_t s_event_sequence;
static uint32_t s_last_status_ms;
static uint32_t s_last_heartbeat_ms;
static uint8_t s_supervisor_seen;
static uint8_t s_chassis_state;
static uint8_t s_fault_code;
static uint32_t s_run_id;
static uint16_t s_last_command_id;
static uint8_t s_question_id;
static uint32_t s_prepared_options;
static uint32_t s_prepared_timeout_ms;
static uint32_t s_stopping_started_ms;
static uint8_t s_stopping_emit_event;
static uint8_t s_a_marker_seen;
static uint8_t s_b_marker_seen;

static uint32_t s_history_run_id;
static uint16_t s_command_history[8];
static uint8_t s_command_history_count;
static uint8_t s_command_history_next;

static uint16_t ring_next(uint16_t index, uint16_t size)
{
    index++;
    return index >= size ? 0U : index;
}

static uint16_t read_u16(const uint8_t *data)
{
    return (uint16_t) data[0] | ((uint16_t) data[1] << 8);
}

static uint32_t read_u32(const uint8_t *data)
{
    return (uint32_t) data[0] |
        ((uint32_t) data[1] << 8) |
        ((uint32_t) data[2] << 16) |
        ((uint32_t) data[3] << 24);
}

static void write_u16(uint8_t *data, uint16_t value)
{
    data[0] = (uint8_t) value;
    data[1] = (uint8_t) (value >> 8);
}

static void write_u32(uint8_t *data, uint32_t value)
{
    data[0] = (uint8_t) value;
    data[1] = (uint8_t) (value >> 8);
    data[2] = (uint8_t) (value >> 16);
    data[3] = (uint8_t) (value >> 24);
}

uint16_t MC02Link_Crc16(const uint8_t *data, uint16_t length)
{
    uint16_t crc = 0xFFFFU;
    for (uint16_t i = 0U; i < length; i++) {
        crc ^= (uint16_t) data[i] << 8;
        for (uint8_t bit = 0U; bit < 8U; bit++) {
            crc = (crc & 0x8000U) != 0U ?
                (uint16_t) ((crc << 1) ^ 0x1021U) :
                (uint16_t) (crc << 1);
        }
    }
    return crc;
}

static void service_tx_fifo(void)
{
    while (s_tx_tail != s_tx_head &&
        !DL_UART_Main_isTXFIFOFull(MC02_UART_INST)) {
        DL_UART_Main_transmitData(
            MC02_UART_INST, s_tx_buffer[s_tx_tail]);
        s_tx_tail = ring_next(s_tx_tail, MC02_TX_BUFFER_SIZE);
    }
    if (s_tx_tail == s_tx_head) {
        DL_UART_Main_disableInterrupt(
            MC02_UART_INST, DL_UART_MAIN_INTERRUPT_TX);
    } else {
        DL_UART_Main_enableInterrupt(
            MC02_UART_INST, DL_UART_MAIN_INTERRUPT_TX);
    }
}

static int queue_bytes(const uint8_t *data, uint16_t length)
{
    uint32_t interrupt_state = __get_PRIMASK();
    __disable_irq();

    uint16_t free_count = s_tx_tail > s_tx_head ?
        (uint16_t) (s_tx_tail - s_tx_head - 1U) :
        (uint16_t) (MC02_TX_BUFFER_SIZE -
            (s_tx_head - s_tx_tail) - 1U);
    if (length > free_count) {
        if (interrupt_state == 0U) {
            __enable_irq();
        }
        return 0;
    }

    for (uint16_t i = 0U; i < length; i++) {
        s_tx_buffer[s_tx_head] = data[i];
        s_tx_head = ring_next(s_tx_head, MC02_TX_BUFFER_SIZE);
    }
    service_tx_fifo();
    if (interrupt_state == 0U) {
        __enable_irq();
    }
    return 1;
}

static int send_frame(
    uint8_t type, const uint8_t *payload, uint16_t payload_length)
{
    uint8_t frame[10U + MC02_MAX_PAYLOAD];
    if (payload_length > MC02_MAX_PAYLOAD) {
        return 0;
    }

    frame[0] = 0xA5U;
    frame[1] = 0x5AU;
    frame[2] = MC02_PROTOCOL_VERSION;
    frame[3] = type;
    write_u16(&frame[4], payload_length);
    write_u16(&frame[6], ++s_tx_sequence);
    for (uint16_t i = 0U; i < payload_length; i++) {
        frame[8U + i] = payload[i];
    }
    uint16_t crc = MC02Link_Crc16(&frame[2], 6U + payload_length);
    write_u16(&frame[8U + payload_length], crc);
    if (!queue_bytes(frame, 10U + payload_length)) {
        return 0;
    }
    g_test.mc02.tx_frames++;
    return 1;
}

static int rx_pop(uint8_t *value)
{
    if (s_rx_tail == s_rx_head) {
        return 0;
    }
    *value = s_rx_buffer[s_rx_tail];
    s_rx_tail = ring_next(s_rx_tail, MC02_RX_BUFFER_SIZE);
    return 1;
}

static int32_t rounded_i32(float value)
{
    if (value >= (float) INT32_MAX) {
        return INT32_MAX;
    }
    if (value <= (float) INT32_MIN) {
        return INT32_MIN;
    }
    return (int32_t) (value >= 0.0f ? value + 0.5f : value - 0.5f);
}

static int16_t rounded_i16(float value)
{
    int32_t rounded = rounded_i32(value);
    if (rounded > INT16_MAX) {
        return INT16_MAX;
    }
    if (rounded < INT16_MIN) {
        return INT16_MIN;
    }
    return (int16_t) rounded;
}

static uint8_t fault_from_test_status(int32_t status)
{
    if (status == TEST_STATUS_LINE_SENSOR_FAULT) {
        return MC02_FAULT_LINE_SENSOR;
    }
    if (status == TEST_STATUS_ENCODER_FAULT) {
        return MC02_FAULT_ENCODER;
    }
    if (status == TEST_STATUS_OUTPUT_FAULT) {
        return MC02_FAULT_OUTPUT;
    }
    if (status == TEST_STATUS_BAD_PARAMETER ||
        status == TEST_STATUS_NOT_ARMED) {
        return MC02_FAULT_BAD_PARAMETER;
    }
    return MC02_FAULT_ROUTE_TIMEOUT;
}

static uint32_t expected_options(uint8_t question_id)
{
    if (question_id == 1U) {
        return MC02_Q1_OPTIONS;
    }
    if (question_id == 2U) {
        return MC02_Q2_OPTIONS;
    }
    if (question_id == 3U) {
        return MC02_Q3_OPTIONS;
    }
    return MC02_Q45_OPTIONS;
}

static uint32_t expected_timeout_ms(uint8_t question_id)
{
    if (question_id == 1U) {
        return MC02_Q1_DEFAULT_TIMEOUT_MS;
    }
    if (question_id == 2U) {
        return MC02_Q2_DEFAULT_TIMEOUT_MS;
    }
    if (question_id == 3U) {
        return MC02_Q3_DEFAULT_TIMEOUT_MS;
    }
    return MC02_Q45_DEFAULT_TIMEOUT_MS;
}

static uint16_t status_flags(void)
{
    uint16_t flags = 0U;
    const LineSensorData *line = LineSensor_GetData();
    if (line->connected != 0U && line->line_lost == 0U) {
        flags |= MC02_FLAG_LINE_SENSOR_OK;
    }
    if ((g_test.encoder_verified_mask & 1U) != 0U) {
        flags |= MC02_FLAG_LEFT_ENCODER_OK;
    }
    if ((g_test.encoder_verified_mask & 2U) != 0U) {
        flags |= MC02_FLAG_RIGHT_ENCODER_OK;
    }
    if (Motor_IsArmed()) {
        flags |= MC02_FLAG_MOTOR_OUTPUT_ENABLED;
    }
    if (s_a_marker_seen != 0U) {
        flags |= MC02_FLAG_A_MARKER_SEEN;
    }
    if (s_b_marker_seen != 0U) {
        flags |= MC02_FLAG_B_MARKER_SEEN;
    }
    if (g_test.mc02.online != 0U) {
        flags |= MC02_FLAG_SUPERVISOR_ONLINE;
    }
    if (s_chassis_state == MC02_STATE_STOPPING ||
        s_chassis_state == MC02_STATE_FAULT) {
        flags |= MC02_FLAG_STOP_LATCHED;
    }
    if (TestMode_IsEmergencyStopLatched()) {
        flags |= MC02_FLAG_EMERGENCY_STOP_LATCHED;
    }
    if (TestMode_IsButtonPanelOk()) {
        flags |= MC02_FLAG_BUTTON_PANEL_OK;
    }
    uint8_t common_motion_phase = TestMode_GetCommonMotionPhase();
    if (common_motion_phase == TEST_COMMON_MOTION_ACCELERATING) {
        flags |= MC02_FLAG_COMMON_ACCEL_ACTIVE;
    } else if (common_motion_phase ==
        TEST_COMMON_MOTION_DECELERATING) {
        flags |= MC02_FLAG_COMMON_DECEL_ACTIVE;
    }
    return flags;
}

static void send_status(void)
{
    uint8_t payload[28];
    write_u32(&payload[0], s_run_id);
    write_u16(&payload[4], s_last_command_id);
    payload[6] = s_chassis_state;
    payload[7] = s_fault_code;
    write_u32(&payload[8], Platform_Millis());
    write_u32(&payload[12], g_test.elapsed_ms);
    write_u32(&payload[16],
        (uint32_t) rounded_i32(g_test.line.lap_distance_mm));
    write_u16(&payload[20],
        (uint16_t) rounded_i16(g_test.left.rpm * 10.0f));
    write_u16(&payload[22],
        (uint16_t) rounded_i16(g_test.right.rpm * 10.0f));
    write_u16(&payload[24],
        (uint16_t) rounded_i16((float) g_test.line.position_x1000));
    write_u16(&payload[26], status_flags());
    (void) send_frame(MC02_MSG_STATUS, payload, sizeof(payload));
}

static void send_event(uint8_t event_code, uint8_t detail)
{
    uint8_t payload[16];
    write_u32(&payload[0], s_run_id);
    write_u16(&payload[4], ++s_event_sequence);
    payload[6] = event_code;
    payload[7] = detail;
    write_u32(&payload[8], g_test.elapsed_ms);
    write_u32(&payload[12],
        (uint32_t) rounded_i32(g_test.line.lap_distance_mm));
    (void) send_frame(MC02_MSG_EVENT, payload, sizeof(payload));
}

static void send_button_event(uint8_t button_id, uint8_t button_action)
{
    uint8_t payload[12];
    write_u16(&payload[0], ++s_event_sequence);
    payload[2] = button_id;
    payload[3] = button_action;
    write_u32(&payload[4], Platform_Millis());
    write_u32(&payload[8], s_run_id);
    (void) send_frame(MC02_MSG_BUTTON, payload, sizeof(payload));
}

static void clear_command_history(uint32_t run_id)
{
    s_history_run_id = run_id;
    s_command_history_count = 0U;
    s_command_history_next = 0U;
}

static int command_seen(uint32_t run_id, uint16_t command_id)
{
    if (run_id != s_history_run_id) {
        return 0;
    }
    for (uint8_t i = 0U; i < s_command_history_count; i++) {
        if (s_command_history[i] == command_id) {
            return 1;
        }
    }
    return 0;
}

static void remember_command(uint32_t run_id, uint16_t command_id)
{
    if (run_id != s_history_run_id) {
        clear_command_history(run_id);
    }
    s_command_history[s_command_history_next] = command_id;
    s_command_history_next =
        (uint8_t) ((s_command_history_next + 1U) % 8U);
    if (s_command_history_count < 8U) {
        s_command_history_count++;
    }
}

static void stop_to_idle(uint8_t emit_event)
{
    TestMode_SupervisorStop();
    if (TestMode_IsEmergencyStopLatched()) {
        s_chassis_state = MC02_STATE_EMERGENCY_STOP_LATCHED;
        s_stopping_emit_event = 0U;
    } else if (s_chassis_state != MC02_STATE_STOPPING) {
        s_chassis_state = MC02_STATE_STOPPING;
        s_stopping_started_ms = Platform_Millis();
        s_stopping_emit_event = emit_event;
    } else if (emit_event != 0U) {
        s_stopping_emit_event = 1U;
    }
    s_fault_code = MC02_FAULT_NONE;
    s_prepared_options = 0U;
    s_prepared_timeout_ms = 0U;
}

static void process_heartbeat(const uint8_t *payload, uint16_t length)
{
    if (length != 8U) {
        return;
    }
    (void) read_u32(&payload[0]);
    s_last_heartbeat_ms = Platform_Millis();
    s_supervisor_seen = 1U;
    g_test.mc02.online = 1U;
}

static void process_command(const uint8_t *payload, uint16_t length)
{
    if (length != 16U) {
        return;
    }

    uint32_t run_id = read_u32(&payload[0]);
    uint16_t command_id = read_u16(&payload[4]);
    uint8_t question_id = payload[6];
    uint8_t action = payload[7];
    uint32_t timeout_ms = read_u32(&payload[8]);
    uint32_t options = read_u32(&payload[12]);

    if (action == MC02_ACTION_SAFE_STOP) {
        if (s_run_id == 0U ||
            s_chassis_state == MC02_STATE_IDLE_DISARMED) {
            s_run_id = run_id;
        }
        s_last_command_id = command_id;
        s_question_id = question_id;
        stop_to_idle(1U);
        return;
    }

    if (command_seen(run_id, command_id)) {
        send_status();
        return;
    }
    if (action > MC02_ACTION_CLEAR_FAULT ||
        question_id < 1U || question_id > 5U) {
        return;
    }

    if (run_id != s_run_id) {
        if (s_chassis_state != MC02_STATE_IDLE_DISARMED ||
            TestMode_IsRunActive()) {
            return;
        }
        s_run_id = run_id;
        clear_command_history(run_id);
    }
    remember_command(run_id, command_id);
    s_last_command_id = command_id;
    s_question_id = question_id;

    if (action == MC02_ACTION_CANCEL) {
        stop_to_idle(1U);
        return;
    }
    if (action == MC02_ACTION_CLEAR_FAULT) {
        if (!TestMode_IsEmergencyStopLatched() &&
            !TestMode_IsRunActive() && !Motor_IsArmed()) {
            stop_to_idle(0U);
        }
        return;
    }

    if (action == MC02_ACTION_PREPARE) {
        if (options == expected_options(question_id) &&
            timeout_ms == expected_timeout_ms(question_id) &&
            g_test.mc02.online != 0U &&
            TestMode_SupervisorPrepareQuestion(question_id)) {
            s_a_marker_seen = 0U;
            s_b_marker_seen = 0U;
            s_prepared_options = options;
            s_prepared_timeout_ms = timeout_ms;
            s_chassis_state = MC02_STATE_READY;
            s_fault_code = MC02_FAULT_NONE;
            send_event(MC02_EVENT_READY, question_id);
        }
        return;
    }

    if (action == MC02_ACTION_START &&
        s_chassis_state == MC02_STATE_READY &&
        options == s_prepared_options &&
        timeout_ms == s_prepared_timeout_ms &&
        TestMode_SupervisorStartQuestion(question_id, timeout_ms)) {
        if (question_id == 2U) {
            QuestionTimer_Start(question_id);
        }
        s_chassis_state = MC02_STATE_RUNNING;
        s_fault_code = MC02_FAULT_NONE;
        send_event(MC02_EVENT_STARTED, question_id);
    }
}

static void process_frame(void)
{
    uint8_t type = s_parse_body[1];
    uint16_t length = read_u16(&s_parse_body[2]);
    const uint8_t *payload = &s_parse_body[6];
    g_test.mc02.rx_frames++;
    if (type == MC02_MSG_HEARTBEAT) {
        process_heartbeat(payload, length);
    } else if (type == MC02_MSG_COMMAND) {
        process_command(payload, length);
    }
}

static void reset_parser(void)
{
    s_parse_state = 0U;
    s_parse_index = 0U;
    s_parse_body_length = 0U;
    s_parse_crc = 0U;
}

static void consume_byte(uint8_t byte)
{
    if (s_parse_state == 0U) {
        if (byte == 0xA5U) {
            s_parse_state = 1U;
        }
        return;
    }
    if (s_parse_state == 1U) {
        if (byte == 0x5AU) {
            s_parse_state = 2U;
            s_parse_index = 0U;
            s_parse_body_length = 0U;
        } else if (byte != 0xA5U) {
            reset_parser();
        }
        return;
    }
    if (s_parse_state == 2U) {
        s_parse_body[s_parse_index++] = byte;
        if (s_parse_index == 4U) {
            uint16_t length = read_u16(&s_parse_body[2]);
            if (s_parse_body[0] != MC02_PROTOCOL_VERSION ||
                length > MC02_MAX_PAYLOAD) {
                reset_parser();
                return;
            }
            s_parse_body_length = 6U + length;
        }
        if (s_parse_body_length != 0U &&
            s_parse_index >= s_parse_body_length) {
            s_parse_state = 3U;
        }
        return;
    }
    if (s_parse_state == 3U) {
        s_parse_crc = byte;
        s_parse_state = 4U;
        return;
    }

    s_parse_crc |= (uint16_t) byte << 8;
    uint16_t expected =
        MC02Link_Crc16(s_parse_body, s_parse_body_length);
    if (s_parse_crc == expected) {
        process_frame();
    } else {
        g_test.mc02.crc_errors++;
    }
    reset_parser();
}

static void update_route_state(uint32_t now)
{
    if (TestMode_IsEmergencyStopLatched()) {
        if (s_chassis_state !=
            MC02_STATE_EMERGENCY_STOP_LATCHED) {
            TestMode_SupervisorStop();
            s_chassis_state =
                MC02_STATE_EMERGENCY_STOP_LATCHED;
            s_fault_code = MC02_FAULT_NONE;
            send_event(MC02_EVENT_EMERGENCY_STOPPED, 8U);
        }
        return;
    }

    if (s_chassis_state == MC02_STATE_STOPPING) {
        if ((uint32_t) (now - s_stopping_started_ms) >=
            MC02_STOPPING_HOLD_MS) {
            s_chassis_state = MC02_STATE_IDLE_DISARMED;
            if (s_stopping_emit_event != 0U) {
                send_event(MC02_EVENT_STOPPED, 0U);
            }
            s_stopping_emit_event = 0U;
        }
        return;
    }

    if (s_supervisor_seen != 0U &&
        (uint32_t) (now - s_last_heartbeat_ms) >
            MC02_HEARTBEAT_TIMEOUT_MS) {
        g_test.mc02.online = 0U;
        if (s_chassis_state == MC02_STATE_RUNNING) {
            TestMode_SupervisorStop();
            s_chassis_state = MC02_STATE_FAULT;
            s_fault_code = MC02_FAULT_SUPERVISOR_LOST;
            send_event(MC02_EVENT_FAULT, s_fault_code);
        } else if (s_chassis_state == MC02_STATE_READY) {
            stop_to_idle(0U);
        }
    }

    if (s_chassis_state != MC02_STATE_RUNNING) {
        return;
    }

    if (s_question_id == 3U &&
        s_b_marker_seen == 0U &&
        (g_test.line.lap_distance_mm >=
             LINE_COURSE_STRAIGHT_MM ||
         g_test.status == TEST_STATUS_COMPLETE)) {
        s_b_marker_seen = 1U;
        send_event(MC02_EVENT_B_MARKER_PASSED, 3U);
    }
    if ((s_question_id == 1U ||
         s_question_id == 4U ||
        s_question_id == 5U) &&
        s_a_marker_seen == 0U &&
        g_test.line.lap_phase >=
            LINE_LAP_PHASE_MARKER_DETECTED) {
        s_a_marker_seen = 1U;
        send_event(MC02_EVENT_A_MARKER_DETECTED, 1U);
    }
    if (g_test.status == TEST_STATUS_COMPLETE ||
        g_test.status == TEST_STATUS_FINISH_DISTANCE_FALLBACK) {
        s_chassis_state = MC02_STATE_ROUTE_COMPLETE;
        send_event(MC02_EVENT_ROUTE_COMPLETE,
            (uint8_t) g_test.status);
    } else if (g_test.status < TEST_STATUS_IDLE) {
        uint8_t fault = fault_from_test_status(g_test.status);
        TestMode_SupervisorStop();
        s_chassis_state = MC02_STATE_FAULT;
        s_fault_code = fault;
        send_event(MC02_EVENT_FAULT, s_fault_code);
    }
}

void MC02Link_Init(void)
{
    s_rx_head = 0U;
    s_rx_tail = 0U;
    s_rx_overflow = 0U;
    s_tx_head = 0U;
    s_tx_tail = 0U;
    reset_parser();
    s_tx_sequence = 0U;
    s_event_sequence = 0U;
    s_last_status_ms = Platform_Millis();
    s_last_heartbeat_ms = s_last_status_ms;
    s_supervisor_seen = 0U;
    s_chassis_state = TestMode_IsEmergencyStopLatched() ?
        MC02_STATE_EMERGENCY_STOP_LATCHED :
        MC02_STATE_IDLE_DISARMED;
    s_fault_code = MC02_FAULT_NONE;
    s_run_id = 0U;
    s_last_command_id = 0U;
    s_question_id = 0U;
    s_prepared_options = 0U;
    s_prepared_timeout_ms = 0U;
    s_stopping_started_ms = 0U;
    s_stopping_emit_event = 0U;
    s_a_marker_seen = 0U;
    s_b_marker_seen = 0U;
    clear_command_history(0U);

    g_test.mc02.online = 0U;
    g_test.mc02.state = s_chassis_state;
    g_test.mc02.fault = s_fault_code;
    g_test.mc02.run_id = 0U;
    g_test.mc02.last_command_id = 0U;
    g_test.mc02.question_id = 0U;
    g_test.mc02.rx_frames = 0U;
    g_test.mc02.crc_errors = 0U;
    g_test.mc02.tx_frames = 0U;

    DL_UART_Main_enableInterrupt(
        MC02_UART_INST, DL_UART_MAIN_INTERRUPT_RX);
    NVIC_ClearPendingIRQ(MC02_UART_INST_INT_IRQN);
    NVIC_EnableIRQ(MC02_UART_INST_INT_IRQN);
}

void MC02Link_Tick(void)
{
    if (s_rx_overflow != 0U) {
        uint32_t interrupt_state = __get_PRIMASK();
        __disable_irq();
        s_rx_tail = s_rx_head;
        s_rx_overflow = 0U;
        if (interrupt_state == 0U) {
            __enable_irq();
        }
        reset_parser();
    }

    uint8_t byte;
    while (rx_pop(&byte)) {
        consume_byte(byte);
    }

    uint32_t now = Platform_Millis();
    update_route_state(now);

    uint8_t button_id;
    uint8_t button_action;
    while (TestMode_TakeButtonEvent(&button_id, &button_action)) {
        if (button_action == 1U && button_id == 8U) {
            s_chassis_state =
                MC02_STATE_EMERGENCY_STOP_LATCHED;
            s_fault_code = MC02_FAULT_NONE;
        } else if (button_action == 1U && button_id == 7U &&
            !TestMode_IsEmergencyStopLatched()) {
            stop_to_idle(1U);
        }
        send_button_event(button_id, button_action);
    }

    if ((uint32_t) (now - s_last_status_ms) >=
        MC02_STATUS_PERIOD_MS) {
        s_last_status_ms = now;
        send_status();
    }

    g_test.mc02.state = s_chassis_state;
    g_test.mc02.fault = s_fault_code;
    g_test.mc02.run_id = s_run_id;
    g_test.mc02.last_command_id = s_last_command_id;
    g_test.mc02.question_id = s_question_id;
}

void MC02_UART_INST_IRQHandler(void)
{
    for (;;) {
        uint32_t interrupt =
            DL_UART_Main_getPendingInterrupt(MC02_UART_INST);
        if (interrupt == DL_UART_MAIN_IIDX_NO_INTERRUPT) {
            break;
        }
        if (interrupt == DL_UART_MAIN_IIDX_RX) {
            while (!DL_UART_Main_isRXFIFOEmpty(MC02_UART_INST)) {
                uint8_t byte =
                    (uint8_t) DL_UART_Main_receiveData(MC02_UART_INST);
                uint16_t next =
                    ring_next(s_rx_head, MC02_RX_BUFFER_SIZE);
                if (next == s_rx_tail) {
                    s_rx_overflow = 1U;
                } else {
                    s_rx_buffer[s_rx_head] = byte;
                    s_rx_head = next;
                }
            }
        } else if (interrupt == DL_UART_MAIN_IIDX_TX) {
            service_tx_fifo();
        }
    }
}
