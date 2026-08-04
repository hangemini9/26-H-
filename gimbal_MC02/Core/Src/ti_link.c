#include "ti_link.h"

#include <string.h>

#include "gimbal_app.h"
#include "gimbal_config.h"
#include "jetson_link.h"
#include "main.h"

#define TI_PROTOCOL_VERSION                 0x02U
#define TI_MAX_PAYLOAD                        64U
#define TI_RX_BUFFER_SIZE                    256U
#define TI_TX_FRAME_COUNT                      8U
#define TI_MAX_FRAME_SIZE             (10U + TI_MAX_PAYLOAD)

#define TI_HEARTBEAT_PERIOD_MS               100U
#define TI_COMMAND_RETRY_MS                  200U
#define TI_STATUS_TIMEOUT_MS                 500U
#define TI_ACCEL_ESTIMATE_WINDOW_MS           50U
#define TI_ACCEL_FILTER_ALPHA               0.65f
#define TI_ACCEL_MAX_M_S2                   0.40f
#define TI_ACCEL_DEADBAND_M_S2              0.05f
#define TI_Q3_ACCEL_COMMAND_FLOOR_M_S2      0.10f
#define TI_Q45_ACCEL_COMMAND_FLOOR_M_S2    0.048f
#define TI_ACCEL_TAIL_MS                     500U
#define TI_WHEEL_DIAMETER_M                 0.065f
#define TI_PI                               3.14159265f
#define TI_START_DELAY_MS                   3000U
#define TI_Q1_TIMEOUT_MS                   40000U
#define TI_Q1_OPTIONS                         0x05UL
#define TI_Q2_TIMEOUT_MS                   30000U
#define TI_Q2_OPTIONS                         0x00UL
#define TI_Q3_TIMEOUT_MS                    8000U
#define TI_Q3_OPTIONS                         0x08UL
#define TI_Q45_TIMEOUT_MS                  30000U
#define TI_Q45_OPTIONS                        0x05UL

#define TI_MSG_HEARTBEAT                      0x20U
#define TI_MSG_COMMAND                        0x21U
#define TI_MSG_STATUS                         0xA0U
#define TI_MSG_EVENT                          0xA1U
#define TI_MSG_BUTTON                         0xA2U

#define TI_ACTION_SAFE_STOP                      0U
#define TI_ACTION_PREPARE                        1U
#define TI_ACTION_START                          2U

#define TI_STATE_IDLE_DISARMED                   1U
#define TI_STATE_READY                           2U
#define TI_STATE_RUNNING                         3U
#define TI_STATE_ROUTE_COMPLETE                  5U
#define TI_STATE_FAULT                           6U
#define TI_STATE_EMERGENCY_STOP_LATCHED          7U

#define TI_FLAG_EMERGENCY_STOP_LATCHED      (1U << 7)
#define TI_FLAG_BUTTON_PANEL_OK              (1U << 9)
#define TI_FLAG_MOTOR_OUTPUT_ENABLED         (1U << 3)
#define TI_FLAG_COMMON_ACCEL_ACTIVE          (1U << 10)
#define TI_FLAG_COMMON_DECEL_ACTIVE          (1U << 11)

#define TI_EVENT_ROUTE_COMPLETE                  4U
#define TI_EVENT_FAULT                           6U

typedef enum {
    TI_SUPERVISOR_IDLE = 0,
    TI_SUPERVISOR_WAIT_GIMBAL = 1,
    TI_SUPERVISOR_WAIT_READY = 2,
    TI_SUPERVISOR_COUNTDOWN = 3,
    TI_SUPERVISOR_WAIT_RUNNING = 4,
    TI_SUPERVISOR_RUNNING = 5,
    TI_SUPERVISOR_STOPPING = 6,
    TI_SUPERVISOR_FAULT = 7,
    TI_SUPERVISOR_WAIT_BALL_SETTLE = 8
} TiSupervisorState;

typedef struct {
    uint8_t bytes[TI_MAX_FRAME_SIZE];
    uint16_t length;
} TiTxFrame;

static UART_HandleTypeDef s_uart;
static volatile uint8_t s_rx_buffer[TI_RX_BUFFER_SIZE];
static volatile uint16_t s_rx_head;
static volatile uint16_t s_rx_tail;
static uint8_t s_rx_byte;

static TiTxFrame s_tx_frames[TI_TX_FRAME_COUNT];
static volatile uint8_t s_tx_head;
static volatile uint8_t s_tx_tail;
static volatile uint8_t s_tx_busy;

static uint8_t s_parse_state;
static uint8_t s_parse_body[6U + TI_MAX_PAYLOAD];
static uint16_t s_parse_index;
static uint16_t s_parse_body_length;
static uint16_t s_parse_crc;

static TiSupervisorState s_state;
static uint16_t s_tx_sequence;
static uint16_t s_command_id;
static uint16_t s_prepare_command_id;
static uint16_t s_start_command_id;
static uint32_t s_run_id;
static uint32_t s_next_run_id;
static uint32_t s_last_heartbeat_ms;
static uint32_t s_last_status_ms;
static uint32_t s_last_command_ms;
static uint32_t s_start_deadline_ms;
static uint32_t s_rx_frames;
static uint32_t s_crc_errors;
static uint32_t s_tx_frames_count;
static uint32_t s_tx_drops;
static uint8_t s_online;
static uint8_t s_chassis_state;
static uint8_t s_chassis_fault;
static uint16_t s_chassis_flags;
static int16_t s_left_rpm_x10;
static int16_t s_right_rpm_x10;
static float s_chassis_velocity_m_s;
static float s_chassis_acceleration_m_s2;
static float s_accel_anchor_velocity_m_s;
static uint32_t s_accel_anchor_ms;
static uint8_t s_accel_anchor_valid;
static uint16_t s_accel_motion_flags;
static uint8_t s_accel_tail_active;
static uint32_t s_accel_tail_start_ms;
static float s_accel_tail_initial_m_s2;
static uint32_t s_route_complete_ms;
static uint32_t s_ball_settle_start_ms;
static uint8_t s_question_id;
static uint8_t s_button_mask;
static uint8_t s_last_button_id;
static uint8_t s_emergency_pressed;
static uint8_t s_emergency_latched;
static uint8_t s_power_enabled;
static uint8_t s_unsupported_question_id;

static uint16_t ring_next(uint16_t index, uint16_t size)
{
    index++;
    return index >= size ? 0U : index;
}

static uint8_t frame_next(uint8_t index)
{
    index++;
    return index >= TI_TX_FRAME_COUNT ? 0U : index;
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

static int16_t read_i16(const uint8_t *data)
{
    return (int16_t)read_u16(data);
}

static float clamp_float(float value, float minimum, float maximum)
{
    if (value < minimum) {
        return minimum;
    }
    if (value > maximum) {
        return maximum;
    }
    return value;
}

static float abs_float(float value)
{
    return value < 0.0f ? -value : value;
}

static float commanded_acceleration_floor_m_s2(void)
{
    return s_question_id == 3U ?
        TI_Q3_ACCEL_COMMAND_FLOOR_M_S2 :
        TI_Q45_ACCEL_COMMAND_FLOOR_M_S2;
}

static void reset_chassis_acceleration(void)
{
    s_chassis_velocity_m_s = 0.0f;
    s_chassis_acceleration_m_s2 = 0.0f;
    s_accel_anchor_velocity_m_s = 0.0f;
    s_accel_anchor_ms = 0U;
    s_accel_anchor_valid = 0U;
    s_accel_motion_flags = 0U;
    s_accel_tail_active = 0U;
    s_accel_tail_start_ms = 0U;
    s_accel_tail_initial_m_s2 = 0.0f;
}

static void update_chassis_acceleration(uint32_t now_ms)
{
    uint16_t motion_flags = s_chassis_flags &
        (TI_FLAG_COMMON_ACCEL_ACTIVE | TI_FLAG_COMMON_DECEL_ACTIVE);
    if (s_chassis_state != TI_STATE_RUNNING ||
        (s_chassis_flags & TI_FLAG_MOTOR_OUTPUT_ENABLED) == 0U ||
        motion_flags ==
            (TI_FLAG_COMMON_ACCEL_ACTIVE | TI_FLAG_COMMON_DECEL_ACTIVE)) {
        reset_chassis_acceleration();
        return;
    }

    if (motion_flags == 0U) {
        if (s_accel_anchor_valid == 0U) {
            reset_chassis_acceleration();
            return;
        }
        if (s_accel_tail_active == 0U) {
            s_accel_tail_active = 1U;
            s_accel_tail_start_ms = now_ms;
            s_accel_tail_initial_m_s2 =
                s_chassis_acceleration_m_s2;
            s_accel_motion_flags = 0U;
        }
        uint32_t tail_elapsed_ms = now_ms - s_accel_tail_start_ms;
        if (tail_elapsed_ms >= TI_ACCEL_TAIL_MS) {
            reset_chassis_acceleration();
        } else {
            s_chassis_acceleration_m_s2 =
                s_accel_tail_initial_m_s2 *
                (1.0f - (float)tail_elapsed_ms /
                    (float)TI_ACCEL_TAIL_MS);
        }
        return;
    }
    s_accel_tail_active = 0U;

    float average_rpm =
        ((float)s_left_rpm_x10 + (float)s_right_rpm_x10) / 20.0f;
    s_chassis_velocity_m_s =
        average_rpm * TI_PI * TI_WHEEL_DIAMETER_M / 60.0f;
    float acceleration_floor =
        commanded_acceleration_floor_m_s2();
    float commanded_acceleration =
        (motion_flags & TI_FLAG_COMMON_ACCEL_ACTIVE) != 0U ?
        acceleration_floor : -acceleration_floor;
    if (s_accel_anchor_valid == 0U ||
        motion_flags != s_accel_motion_flags) {
        /*
         * TI deliberately slews common wheel speed at a question-specific
         * bounded rate. Apply the corresponding acceleration immediately
         * instead of leaving the first 50 ms (or a noisy low-speed interval)
         * without feed-forward.
         * Wheel-RPM differentiation can still raise the estimate, but never
         * remove the known commanded ramp while its phase flag is active.
         */
        s_chassis_acceleration_m_s2 = commanded_acceleration;
        s_accel_anchor_velocity_m_s = s_chassis_velocity_m_s;
        s_accel_anchor_ms = now_ms;
        s_accel_anchor_valid = 1U;
        s_accel_motion_flags = motion_flags;
        return;
    }

    uint32_t elapsed_ms = now_ms - s_accel_anchor_ms;
    if (elapsed_ms < TI_ACCEL_ESTIMATE_WINDOW_MS) {
        return;
    }
    float raw_acceleration =
        (s_chassis_velocity_m_s - s_accel_anchor_velocity_m_s) *
        (1000.0f / (float)elapsed_ms);
    raw_acceleration = clamp_float(raw_acceleration,
                                   -TI_ACCEL_MAX_M_S2,
                                   TI_ACCEL_MAX_M_S2);
    if (((motion_flags & TI_FLAG_COMMON_ACCEL_ACTIVE) != 0U &&
         raw_acceleration < 0.0f) ||
        ((motion_flags & TI_FLAG_COMMON_DECEL_ACTIVE) != 0U &&
         raw_acceleration > 0.0f) ||
        abs_float(raw_acceleration) < TI_ACCEL_DEADBAND_M_S2 ||
        abs_float(raw_acceleration) < acceleration_floor) {
        raw_acceleration = commanded_acceleration;
    }
    s_chassis_acceleration_m_s2 +=
        TI_ACCEL_FILTER_ALPHA *
        (raw_acceleration - s_chassis_acceleration_m_s2);
    s_accel_anchor_velocity_m_s = s_chassis_velocity_m_s;
    s_accel_anchor_ms = now_ms;
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

uint16_t ti_link_crc16(const uint8_t *data, uint16_t length)
{
    uint16_t crc = 0xFFFFU;
    for (uint16_t i = 0U; i < length; i++) {
        crc ^= (uint16_t)data[i] << 8;
        for (uint8_t bit = 0U; bit < 8U; bit++) {
            crc = (crc & 0x8000U) != 0U ?
                (uint16_t)((crc << 1) ^ 0x1021U) :
                (uint16_t)(crc << 1);
        }
    }
    return crc;
}

static void uart_init(void)
{
    RCC_PeriphCLKInitTypeDef clock = {0};
    GPIO_InitTypeDef gpio = {0};

    clock.PeriphClockSelection = RCC_PERIPHCLK_USART1;
    clock.Usart16ClockSelection = RCC_USART16910CLKSOURCE_D2PCLK2;
    if (HAL_RCCEx_PeriphCLKConfig(&clock) != HAL_OK) {
        Error_Handler();
    }
    __HAL_RCC_USART1_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();

    gpio.Pin = GPIO_PIN_9 | GPIO_PIN_10;
    gpio.Mode = GPIO_MODE_AF_PP;
    gpio.Pull = GPIO_NOPULL;
    gpio.Speed = GPIO_SPEED_FREQ_LOW;
    gpio.Alternate = GPIO_AF7_USART1;
    HAL_GPIO_Init(GPIOA, &gpio);

    s_uart.Instance = USART1;
    s_uart.Init.BaudRate = 115200;
    s_uart.Init.WordLength = UART_WORDLENGTH_8B;
    s_uart.Init.StopBits = UART_STOPBITS_1;
    s_uart.Init.Parity = UART_PARITY_NONE;
    s_uart.Init.Mode = UART_MODE_TX_RX;
    s_uart.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    s_uart.Init.OverSampling = UART_OVERSAMPLING_16;
    s_uart.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
    s_uart.Init.ClockPrescaler = UART_PRESCALER_DIV1;
    s_uart.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
    if (HAL_UART_Init(&s_uart) != HAL_OK ||
        HAL_UARTEx_SetTxFifoThreshold(
            &s_uart, UART_TXFIFO_THRESHOLD_1_8) != HAL_OK ||
        HAL_UARTEx_SetRxFifoThreshold(
            &s_uart, UART_RXFIFO_THRESHOLD_1_8) != HAL_OK ||
        HAL_UARTEx_DisableFifoMode(&s_uart) != HAL_OK) {
        Error_Handler();
    }

    HAL_NVIC_SetPriority(USART1_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(USART1_IRQn);
}

static void kick_tx(void)
{
    if (s_tx_busy != 0U || s_tx_tail == s_tx_head) {
        return;
    }
    s_tx_busy = 1U;
    TiTxFrame *frame = &s_tx_frames[s_tx_tail];
    if (HAL_UART_Transmit_IT(
            &s_uart, frame->bytes, frame->length) != HAL_OK) {
        s_tx_busy = 0U;
        s_tx_tail = frame_next(s_tx_tail);
        s_tx_drops++;
    }
}

static int queue_frame(
    uint8_t type, const uint8_t *payload, uint16_t payload_length)
{
    if (payload_length > TI_MAX_PAYLOAD) {
        return 0;
    }

    uint32_t interrupt_state = __get_PRIMASK();
    __disable_irq();
    uint8_t next = frame_next(s_tx_head);
    if (next == s_tx_tail) {
        s_tx_drops++;
        if (interrupt_state == 0U) {
            __enable_irq();
        }
        return 0;
    }

    TiTxFrame *frame = &s_tx_frames[s_tx_head];
    frame->bytes[0] = 0xA5U;
    frame->bytes[1] = 0x5AU;
    frame->bytes[2] = TI_PROTOCOL_VERSION;
    frame->bytes[3] = type;
    write_u16(&frame->bytes[4], payload_length);
    write_u16(&frame->bytes[6], ++s_tx_sequence);
    if (payload_length != 0U) {
        memcpy(&frame->bytes[8], payload, payload_length);
    }
    uint16_t crc =
        ti_link_crc16(&frame->bytes[2], 6U + payload_length);
    write_u16(&frame->bytes[8U + payload_length], crc);
    frame->length = 10U + payload_length;
    s_tx_head = next;
    s_tx_frames_count++;
    kick_tx();
    if (interrupt_state == 0U) {
        __enable_irq();
    }
    return 1;
}

static void send_heartbeat(uint32_t now_ms)
{
    uint8_t payload[8];
    write_u32(&payload[0], now_ms);
    write_u32(&payload[4], s_run_id);
    (void)queue_frame(TI_MSG_HEARTBEAT, payload, sizeof(payload));
}

static void send_command(
    uint8_t action, uint16_t command_id, uint8_t question_id)
{
    uint8_t payload[16];
    uint32_t timeout_ms;
    uint32_t options;

    if (question_id == 1U) {
        timeout_ms = TI_Q1_TIMEOUT_MS;
        options = TI_Q1_OPTIONS;
    } else if (question_id == 2U) {
        timeout_ms = TI_Q2_TIMEOUT_MS;
        options = TI_Q2_OPTIONS;
    } else if (question_id == 3U) {
        timeout_ms = TI_Q3_TIMEOUT_MS;
        options = TI_Q3_OPTIONS;
    } else {
        timeout_ms = TI_Q45_TIMEOUT_MS;
        options = TI_Q45_OPTIONS;
    }
    write_u32(&payload[0], s_run_id);
    write_u16(&payload[4], command_id);
    payload[6] = question_id;
    payload[7] = action;
    write_u32(&payload[8], timeout_ms);
    write_u32(&payload[12], options);
    (void)queue_frame(TI_MSG_COMMAND, payload, sizeof(payload));
}

static void force_stop(uint32_t now_ms, GimbalTiStopMode stop_mode)
{
    s_route_complete_ms = 0U;
    s_ball_settle_start_ms = 0U;
    s_power_enabled = 0U;
    gimbal_app_force_ti_safe_stop(now_ms, stop_mode);
    if (s_run_id == 0U) {
        s_run_id = s_next_run_id++;
    }
    s_command_id++;
    send_command(TI_ACTION_SAFE_STOP, s_command_id, s_question_id);
    s_last_command_ms = now_ms;
    s_state = stop_mode != GIMBAL_TI_STOP_NORMAL ?
        TI_SUPERVISOR_FAULT : TI_SUPERVISOR_STOPPING;
}

static void begin_question(uint8_t question_id, uint32_t now_ms)
{
    s_run_id = s_next_run_id++;
    if (s_run_id == 0U) {
        s_run_id = s_next_run_id++;
    }
    s_question_id = question_id;
    s_route_complete_ms = 0U;
    s_ball_settle_start_ms = 0U;
    s_unsupported_question_id = 0U;
    if (gimbal_app_begin_question(
            s_run_id, question_id, now_ms) == 0U) {
        force_stop(now_ms, GIMBAL_TI_STOP_LINK_FAULT);
        return;
    }
    jetson_link_send_event(
        s_run_id, 1U, question_id, now_ms);
    s_state = TI_SUPERVISOR_WAIT_GIMBAL;
}

static void finish_route_completion(uint32_t now_ms)
{
    gimbal_app_complete_question(s_run_id, now_ms);
    jetson_link_send_event(
        s_run_id, 5U, s_question_id, now_ms);
    force_stop(now_ms, GIMBAL_TI_STOP_NORMAL);
}

static void accept_route_complete(uint32_t now_ms)
{
    jetson_link_send_event(
        s_run_id, 4U, s_question_id, now_ms);
    if (s_question_id >= 3U && s_question_id <= 5U) {
        s_route_complete_ms = now_ms;
        s_ball_settle_start_ms = 0U;
        s_state = TI_SUPERVISOR_WAIT_BALL_SETTLE;
    } else {
        finish_route_completion(now_ms);
    }
}

static void process_status(
    const uint8_t *payload, uint16_t length, uint32_t now_ms)
{
    if (length != 28U) {
        return;
    }
    uint32_t run_id = read_u32(&payload[0]);
    uint16_t last_command_id = read_u16(&payload[4]);
    s_chassis_state = payload[6];
    s_chassis_fault = payload[7];
    s_left_rpm_x10 = read_i16(&payload[20]);
    s_right_rpm_x10 = read_i16(&payload[22]);
    s_chassis_flags = read_u16(&payload[26]);
    s_last_status_ms = now_ms;
    s_online = 1U;
    update_chassis_acceleration(now_ms);

    if (s_chassis_state == TI_STATE_EMERGENCY_STOP_LATCHED ||
        (s_chassis_flags & TI_FLAG_EMERGENCY_STOP_LATCHED) != 0U) {
        s_emergency_latched = 1U;
        if (s_state != TI_SUPERVISOR_FAULT) {
            force_stop(now_ms, GIMBAL_TI_STOP_LINK_FAULT);
        }
        return;
    }

    if (s_state == TI_SUPERVISOR_IDLE ||
        run_id != s_run_id) {
        return;
    }
    if (s_chassis_state == TI_STATE_FAULT ||
        s_chassis_fault != 0U) {
        if (s_state != TI_SUPERVISOR_STOPPING &&
            s_state != TI_SUPERVISOR_FAULT) {
            force_stop(now_ms, GIMBAL_TI_STOP_LINK_FAULT);
        }
        return;
    }
    if (s_state == TI_SUPERVISOR_WAIT_READY &&
        last_command_id == s_prepare_command_id &&
        s_chassis_state == TI_STATE_READY) {
        s_start_deadline_ms = now_ms + TI_START_DELAY_MS;
        s_state = TI_SUPERVISOR_COUNTDOWN;
        jetson_link_send_event(
            s_run_id, 2U, s_question_id, now_ms);
    } else if (s_state == TI_SUPERVISOR_WAIT_RUNNING &&
        last_command_id == s_start_command_id &&
        s_chassis_state == TI_STATE_RUNNING) {
        s_state = TI_SUPERVISOR_RUNNING;
        gimbal_app_start_question(s_run_id, now_ms);
        jetson_link_send_event(
            s_run_id, 3U, s_question_id, now_ms);
    } else if (s_state == TI_SUPERVISOR_RUNNING &&
        s_chassis_state == TI_STATE_ROUTE_COMPLETE) {
        accept_route_complete(now_ms);
    } else if ((s_state == TI_SUPERVISOR_STOPPING ||
                s_state == TI_SUPERVISOR_FAULT) &&
        s_chassis_state == TI_STATE_IDLE_DISARMED) {
        s_state = TI_SUPERVISOR_IDLE;
        s_run_id = 0U;
        s_question_id = 0U;
    }
}

static void process_event(
    const uint8_t *payload, uint16_t length, uint32_t now_ms)
{
    if (length != 16U || read_u32(&payload[0]) != s_run_id) {
        return;
    }
    uint8_t event_code = payload[6];
    if (event_code == TI_EVENT_ROUTE_COMPLETE &&
        s_state == TI_SUPERVISOR_RUNNING) {
        accept_route_complete(now_ms);
    } else if (event_code == TI_EVENT_FAULT &&
        s_state != TI_SUPERVISOR_STOPPING &&
        s_state != TI_SUPERVISOR_FAULT) {
        force_stop(now_ms, GIMBAL_TI_STOP_LINK_FAULT);
    }
}

static void process_button(
    const uint8_t *payload, uint16_t length, uint32_t now_ms)
{
    if (length != 12U) {
        return;
    }
    uint8_t button_id = payload[2];
    uint8_t action = payload[3];
    if (button_id < 1U || button_id > 8U ||
        (action != 1U && action != 2U)) {
        return;
    }

    uint8_t bit = (uint8_t)(1U << (button_id - 1U));
    if (action == 1U) {
        s_button_mask |= bit;
    } else {
        s_button_mask &= (uint8_t)~bit;
    }
    s_last_button_id = button_id;

    if (button_id == 8U) {
        s_emergency_pressed = action == 1U ? 1U : 0U;
        if (action == 1U) {
            s_emergency_latched = 1U;
            s_power_enabled = 0U;
            jetson_link_send_event(
                s_run_id, 7U, 8U, now_ms);
            force_stop(now_ms, GIMBAL_TI_STOP_LINK_FAULT);
        }
        return;
    }
    if (action != 1U) {
        return;
    }
    if (button_id == 7U) {
        s_power_enabled = 0U;
        if (s_emergency_pressed == 0U) {
            s_emergency_latched = 0U;
            jetson_link_send_event(
                s_run_id, 6U, 7U, now_ms);
            force_stop(now_ms, GIMBAL_TI_STOP_NORMAL);
        } else {
            force_stop(now_ms, GIMBAL_TI_STOP_LINK_FAULT);
        }
        return;
    }
    if (button_id == 6U) {
        if (s_power_enabled != 0U) {
            s_power_enabled = 0U;
            if (s_state != TI_SUPERVISOR_IDLE) {
                force_stop(now_ms, GIMBAL_TI_STOP_NORMAL);
            }
        } else if (s_state == TI_SUPERVISOR_IDLE &&
            s_emergency_latched == 0U) {
            s_power_enabled = 1U;
        }
        return;
    }
    if (s_emergency_latched != 0U) {
        return;
    }
    if (button_id >= 1U && button_id <= 5U &&
        s_state == TI_SUPERVISOR_IDLE) {
        if (button_id == 2U) {
            s_power_enabled = 0U;
            begin_question(button_id, now_ms);
        } else if (s_power_enabled != 0U) {
            begin_question(button_id, now_ms);
        }
    }
}

static void process_frame(uint32_t now_ms)
{
    uint8_t type = s_parse_body[1];
    uint16_t length = read_u16(&s_parse_body[2]);
    const uint8_t *payload = &s_parse_body[6];
    s_rx_frames++;
    if (type == TI_MSG_STATUS) {
        process_status(payload, length, now_ms);
    } else if (type == TI_MSG_EVENT) {
        process_event(payload, length, now_ms);
    } else if (type == TI_MSG_BUTTON) {
        process_button(payload, length, now_ms);
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
            if (s_parse_body[0] != TI_PROTOCOL_VERSION ||
                length > TI_MAX_PAYLOAD) {
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

    s_parse_crc |= (uint16_t)byte << 8;
    if (s_parse_crc ==
        ti_link_crc16(s_parse_body, s_parse_body_length)) {
        process_frame(now_ms);
    } else {
        s_crc_errors++;
    }
    reset_parser();
}

static int rx_pop(uint8_t *byte)
{
    if (s_rx_tail == s_rx_head) {
        return 0;
    }
    *byte = s_rx_buffer[s_rx_tail];
    s_rx_tail = ring_next(s_rx_tail, TI_RX_BUFFER_SIZE);
    return 1;
}

static void update_debug(uint32_t now_ms)
{
    g_gimbal_debug.ti_online = s_online;
    g_gimbal_debug.ti_supervisor_state = (uint32_t)s_state;
    g_gimbal_debug.ti_chassis_state = s_chassis_state;
    g_gimbal_debug.ti_chassis_fault = s_chassis_fault;
    g_gimbal_debug.ti_chassis_flags = s_chassis_flags;
    g_gimbal_debug.ti_run_id = s_run_id;
    g_gimbal_debug.ti_question_id = s_question_id;
    g_gimbal_debug.ti_button_mask = s_button_mask;
    g_gimbal_debug.ti_last_button_id = s_last_button_id;
    g_gimbal_debug.ti_emergency_stop_latched =
        s_emergency_latched;
    g_gimbal_debug.ti_unsupported_question_id =
        s_unsupported_question_id;
    g_gimbal_debug.ti_left_rpm_x10 = s_left_rpm_x10;
    g_gimbal_debug.ti_right_rpm_x10 = s_right_rpm_x10;
    g_gimbal_debug.ti_chassis_accel_mm_s2 =
        (int32_t)(s_chassis_acceleration_m_s2 * 1000.0f);
    g_gimbal_debug.ti_chassis_accel_tail_remaining_ms =
        s_accel_tail_active != 0U &&
        (uint32_t)(now_ms - s_accel_tail_start_ms) < TI_ACCEL_TAIL_MS ?
        TI_ACCEL_TAIL_MS -
            (uint32_t)(now_ms - s_accel_tail_start_ms) : 0U;
    g_gimbal_debug.ti_route_complete_waiting =
        s_state == TI_SUPERVISOR_WAIT_BALL_SETTLE ? 1U : 0U;
    g_gimbal_debug.ti_route_hold_remaining_ms =
        s_state == TI_SUPERVISOR_WAIT_BALL_SETTLE &&
        (uint32_t)(now_ms - s_route_complete_ms) <
            GIMBAL_ROUTE_POST_STOP_HOLD_MS ?
        GIMBAL_ROUTE_POST_STOP_HOLD_MS -
            (uint32_t)(now_ms - s_route_complete_ms) : 0U;
    g_gimbal_debug.ti_ball_settle_elapsed_ms =
        s_ball_settle_start_ms != 0U ?
        now_ms - s_ball_settle_start_ms : 0U;
    g_gimbal_debug.ti_rx_frames = s_rx_frames;
    g_gimbal_debug.ti_crc_errors = s_crc_errors;
    g_gimbal_debug.ti_tx_frames = s_tx_frames_count;
    g_gimbal_debug.ti_tx_drops = s_tx_drops;
}

void ti_link_init(uint32_t now_ms)
{
    uart_init();
    s_rx_head = 0U;
    s_rx_tail = 0U;
    s_tx_head = 0U;
    s_tx_tail = 0U;
    s_tx_busy = 0U;
    reset_parser();
    s_state = TI_SUPERVISOR_IDLE;
    s_tx_sequence = 0U;
    s_command_id = 0U;
    s_prepare_command_id = 0U;
    s_start_command_id = 0U;
    s_run_id = 0U;
    s_next_run_id = 1U;
    s_last_heartbeat_ms = now_ms;
    s_last_status_ms = now_ms;
    s_last_command_ms = now_ms;
    s_start_deadline_ms = now_ms;
    s_rx_frames = 0U;
    s_crc_errors = 0U;
    s_tx_frames_count = 0U;
    s_tx_drops = 0U;
    s_online = 0U;
    s_chassis_state = 0U;
    s_chassis_fault = 0U;
    s_chassis_flags = 0U;
    s_left_rpm_x10 = 0;
    s_right_rpm_x10 = 0;
    reset_chassis_acceleration();
    s_route_complete_ms = 0U;
    s_ball_settle_start_ms = 0U;
    s_question_id = 0U;
    s_button_mask = 0U;
    s_last_button_id = 0U;
    s_emergency_pressed = 0U;
    s_emergency_latched = 0U;
    s_power_enabled = 0U;
    s_unsupported_question_id = 0U;
    (void)HAL_UART_Receive_IT(&s_uart, &s_rx_byte, 1U);
    send_heartbeat(now_ms);
    update_debug(now_ms);
}

void ti_link_poll(uint32_t now_ms)
{
    uint8_t byte;
    while (rx_pop(&byte)) {
        consume_byte(byte, now_ms);
    }

    if ((uint32_t)(now_ms - s_last_heartbeat_ms) >=
        TI_HEARTBEAT_PERIOD_MS) {
        s_last_heartbeat_ms = now_ms;
        send_heartbeat(now_ms);
    }
    if ((uint32_t)(now_ms - s_last_status_ms) >
        TI_STATUS_TIMEOUT_MS) {
        s_online = 0U;
        reset_chassis_acceleration();
        if (s_state != TI_SUPERVISOR_IDLE &&
            s_state != TI_SUPERVISOR_STOPPING &&
            s_state != TI_SUPERVISOR_FAULT) {
            force_stop(now_ms, GIMBAL_TI_STOP_LINK_FAULT);
        }
    }

    if (s_state == TI_SUPERVISOR_WAIT_GIMBAL) {
        if ((gimbal_app_question_faulted(s_run_id) != 0U) ||
            (gimbal_app_stop_latched() != 0U)) {
            jetson_link_send_event(
                s_run_id, 7U, s_question_id, now_ms);
            force_stop(
                now_ms, GIMBAL_TI_STOP_PRESERVE_FAULT);
        } else if (gimbal_app_question_ready(s_run_id) != 0U) {
            s_prepare_command_id = ++s_command_id;
            send_command(
                TI_ACTION_PREPARE,
                s_prepare_command_id,
                s_question_id);
            s_last_command_ms = now_ms;
            s_state = TI_SUPERVISOR_WAIT_READY;
        }
    }

    if ((s_state == TI_SUPERVISOR_RUNNING ||
         s_state == TI_SUPERVISOR_WAIT_BALL_SETTLE) &&
        ((gimbal_app_question_faulted(s_run_id) != 0U) ||
         (gimbal_app_stop_latched() != 0U))) {
        jetson_link_send_event(
            s_run_id, 7U, s_question_id, now_ms);
        force_stop(
            now_ms, GIMBAL_TI_STOP_PRESERVE_FAULT);
    } else if (s_state == TI_SUPERVISOR_RUNNING &&
        s_question_id == 2U &&
        gimbal_app_question_complete(s_run_id) != 0U) {
        gimbal_app_complete_question(s_run_id, now_ms);
        jetson_link_send_event(
            s_run_id, 5U, s_question_id, now_ms);
        force_stop(now_ms, GIMBAL_TI_STOP_NORMAL);
    }

    if (s_state == TI_SUPERVISOR_WAIT_BALL_SETTLE) {
        if (gimbal_app_ball_within_route_settle_limits(now_ms) != 0U) {
            if (s_ball_settle_start_ms == 0U) {
                s_ball_settle_start_ms = now_ms;
            } else if ((uint32_t)(now_ms - s_ball_settle_start_ms) >=
                           GIMBAL_ROUTE_SETTLE_MS &&
                       (uint32_t)(now_ms - s_route_complete_ms) >=
                           GIMBAL_ROUTE_POST_STOP_HOLD_MS) {
                finish_route_completion(now_ms);
            }
        } else {
            s_ball_settle_start_ms = 0U;
        }

        if (s_state == TI_SUPERVISOR_WAIT_BALL_SETTLE &&
            (uint32_t)(now_ms - s_route_complete_ms) >=
                GIMBAL_ROUTE_SETTLE_TIMEOUT_MS) {
            jetson_link_send_event(
                s_run_id, 7U, s_question_id, now_ms);
            gimbal_app_stop_question(s_run_id, 0U, now_ms);
            force_stop(now_ms, GIMBAL_TI_STOP_NORMAL);
        }
    }

    if (s_state == TI_SUPERVISOR_COUNTDOWN &&
        (int32_t)(now_ms - s_start_deadline_ms) >= 0) {
        s_start_command_id = ++s_command_id;
        send_command(
            TI_ACTION_START, s_start_command_id, s_question_id);
        s_last_command_ms = now_ms;
        s_state = TI_SUPERVISOR_WAIT_RUNNING;
    }

    if ((s_state == TI_SUPERVISOR_WAIT_READY ||
            s_state == TI_SUPERVISOR_WAIT_RUNNING) &&
        (uint32_t)(now_ms - s_last_command_ms) >=
            TI_COMMAND_RETRY_MS) {
        s_last_command_ms = now_ms;
        if (s_state == TI_SUPERVISOR_WAIT_READY) {
            send_command(
                TI_ACTION_PREPARE,
                s_prepare_command_id,
                s_question_id);
        } else {
            send_command(
                TI_ACTION_START,
                s_start_command_id,
                s_question_id);
        }
    } else if ((s_state == TI_SUPERVISOR_STOPPING ||
                s_state == TI_SUPERVISOR_FAULT) &&
        (uint32_t)(now_ms - s_last_command_ms) >= 100U) {
        s_last_command_ms = now_ms;
        send_command(
            TI_ACTION_SAFE_STOP, s_command_id, s_question_id);
    }
    update_debug(now_ms);
}

void ti_link_uart_irq_handler(void)
{
    HAL_UART_IRQHandler(&s_uart);
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *uart)
{
    if (uart->Instance != USART1) {
        return;
    }
    uint16_t next = ring_next(s_rx_head, TI_RX_BUFFER_SIZE);
    if (next != s_rx_tail) {
        s_rx_buffer[s_rx_head] = s_rx_byte;
        s_rx_head = next;
    }
    (void)HAL_UART_Receive_IT(&s_uart, &s_rx_byte, 1U);
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *uart)
{
    if (uart->Instance != USART1) {
        return;
    }
    s_tx_tail = frame_next(s_tx_tail);
    s_tx_busy = 0U;
    kick_tx();
}

void HAL_UART_ErrorCallback(UART_HandleTypeDef *uart)
{
    if (uart->Instance != USART1) {
        return;
    }
    __HAL_UART_CLEAR_OREFLAG(&s_uart);
    (void)HAL_UART_Receive_IT(&s_uart, &s_rx_byte, 1U);
}

uint32_t ti_link_run_id(void)
{
    return s_run_id;
}

uint8_t ti_link_question_id(void)
{
    return s_question_id;
}

uint8_t ti_link_chassis_online(void)
{
    return s_online;
}

uint8_t ti_link_supervisor_state(void)
{
    return (uint8_t)s_state;
}

float ti_link_chassis_acceleration_m_s2(void)
{
    return s_online != 0U ? s_chassis_acceleration_m_s2 : 0.0f;
}
