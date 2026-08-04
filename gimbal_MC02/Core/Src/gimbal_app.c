#include "gimbal_app.h"
#include "dm4310.h"
#include "fdcan.h"
#include "gimbal_config.h"
#include "jetson_link.h"
#include "main.h"
#include "ti_link.h"
#include "usb_console.h"
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#define DEG_TO_RAD 0.01745329251994329577f

volatile GimbalDebug g_gimbal_debug;

typedef enum
{
    CALRUN_IDLE = 0,
    CALRUN_WAIT_READY = 1,
    CALRUN_COUNTDOWN = 2,
    CALRUN_POSITIVE = 3,
    CALRUN_RETURN_FROM_POSITIVE = 4,
    CALRUN_NEGATIVE = 5,
    CALRUN_FINAL_RETURN = 6
} CalRunPhase;

typedef enum
{
    COMPETITION_IDLE = 0,
    COMPETITION_WAIT_POWER = 1,
    COMPETITION_WAIT_VISION = 2,
    COMPETITION_READY = 3,
    COMPETITION_RUNNING = 4,
    COMPETITION_Q2_POSITIVE = 5,
    COMPETITION_Q2_NEGATIVE = 6,
    COMPETITION_COMPLETE = 7,
    COMPETITION_FAULT = 8
} CompetitionPhase;

#if GIMBAL_ENABLE_DELAY_TEST
typedef enum
{
    DELAY_TEST_IDLE = 0,
    DELAY_TEST_WAIT_POWER = 1,
    DELAY_TEST_COUNTDOWN = 2,
    DELAY_TEST_READY_VALIDATE = 3,
    DELAY_TEST_PULSE = 4,
    DELAY_TEST_RETURNING = 5,
    DELAY_TEST_WAIT_ACK = 6,
    DELAY_TEST_READY_MEASURE = 7,
    DELAY_TEST_FAULT = 8
} DelayTestPhase;
#endif

static GimbalState s_state;
static GimbalFault s_fault;
static uint32_t s_state_start_ms;
static uint32_t s_arm_deadline_ms;
static uint32_t s_power_arm_deadline_ms;
static uint32_t s_power_off_deadline_ms;
static uint32_t s_post_jog_disable_deadline_ms;
static uint32_t s_next_motor_tx_ms;
static uint8_t s_disable_burst;
static uint8_t s_power_output_enabled;
static uint8_t s_level_zero_valid;
static uint8_t s_zero_referenced_motion;
static CalRunPhase s_calrun_phase;
static uint32_t s_calrun_deadline_ms;
static int32_t s_calrun_amplitude_tenths_deg;
static float s_level_zero_rad;
static float s_neutral_rad;
static float s_requested_rad;
static float s_command_rad;
static float s_sweep_amplitude_rad;
static int8_t s_sweep_direction;
static VisionSample s_vision;
static uint8_t s_vision_received;
static CompetitionPhase s_competition_phase;
static uint32_t s_competition_run_id;
static uint32_t s_competition_start_ms;
static uint32_t s_target_settle_start_ms;
static uint32_t s_q2_breakaway_deadline_ms;
static uint32_t s_q2_breakaway_count;
static int32_t s_q2_breakaway_pipe_mdeg;
static uint8_t s_q2_breakaway_used_this_leg;
static uint32_t s_q2_stall_start_ms;
static uint32_t s_q2_stall_last_sample_ms;
static int32_t s_q2_stall_min_um;
static int32_t s_q2_stall_max_um;
static uint8_t s_q2_final_capture_latched;
static uint8_t s_competition_question_id;
static int32_t s_competition_target_um;
static int32_t s_command_pipe_angle_mdeg;
static int32_t
    s_velocity_history_um[GIMBAL_VELOCITY_HISTORY_DEPTH];
static uint32_t
    s_velocity_history_ms[GIMBAL_VELOCITY_HISTORY_DEPTH];
static uint32_t s_velocity_history_count;
static uint32_t s_velocity_history_head;
static uint32_t s_velocity_last_sample_ms;
static uint32_t s_velocity_window_ms;
static int32_t s_velocity_filtered_um_s;
static uint32_t s_q2_motion_last_sample_ms;
static uint32_t s_q2_motion_window_start_ms;
static int32_t s_q2_motion_min_um;
static int32_t s_q2_motion_max_um;
static uint32_t s_q2_stationary_ms;
static uint32_t s_control_prediction_horizon_ms;
static int32_t s_control_predicted_position_um;
static int32_t s_control_velocity_um_s;
#if GIMBAL_ENABLE_DELAY_TEST
static DelayTestPhase s_delay_test_phase;
static uint32_t s_delay_test_deadline_ms;
static uint32_t s_delay_test_arm_deadline_ms;
static uint32_t s_delay_test_return_settle_start_ms;
static uint8_t s_delay_test_validated;
static uint8_t s_delay_test_mechanical_ack;
static uint8_t s_delay_test_measurement_active;
static uint32_t s_delay_test_pulse_count;
static uint32_t s_delay_test_measurement_count;
static uint32_t s_delay_test_denied_count;
static uint32_t s_delay_test_last_denied_request;
static uint32_t s_delay_test_last_pulse_start_ms;
static uint32_t s_delay_test_stable_start_ms;
static uint32_t s_delay_test_stable_last_sample_ms;
static int32_t s_delay_test_stable_min_um;
static int32_t s_delay_test_stable_max_um;
static uint32_t s_delay_test_stable_sample_count;
static uint8_t s_delay_test_stable_ready;

static void delay_test_stability_reset(void);
#endif

static uint8_t time_reached(uint32_t now_ms, uint32_t deadline_ms)
{
    return ((int32_t)(now_ms - deadline_ms) >= 0) ? 1U : 0U;
}

static void power_output_force_off(void)
{
    HAL_GPIO_WritePin(POWER_24V_1_GPIO_Port,
                      POWER_24V_1_Pin,
                      GPIO_PIN_RESET);
    s_power_output_enabled = 0U;
    s_power_arm_deadline_ms = 0U;
    s_power_off_deadline_ms = 0U;
    s_post_jog_disable_deadline_ms = 0U;
    s_level_zero_valid = 0U;
    s_level_zero_rad = 0.0f;
    s_zero_referenced_motion = 0U;
    s_calrun_phase = CALRUN_IDLE;
    s_calrun_deadline_ms = 0U;
#if GIMBAL_ENABLE_DELAY_TEST
    s_delay_test_phase = DELAY_TEST_IDLE;
    s_delay_test_deadline_ms = 0U;
    s_delay_test_arm_deadline_ms = 0U;
    s_delay_test_return_settle_start_ms = 0U;
    s_delay_test_validated = 0U;
    s_delay_test_mechanical_ack = 0U;
    s_delay_test_measurement_active = 0U;
    delay_test_stability_reset();
#endif
}

static uint8_t power_arm_is_valid(uint32_t now_ms)
{
    if (s_power_arm_deadline_ms == 0U)
    {
        return 0U;
    }
    return (time_reached(now_ms, s_power_arm_deadline_ms) == 0U) ? 1U : 0U;
}

static uint8_t power_output_is_ready(void)
{
    return ((s_power_output_enabled != 0U) &&
            (s_disable_burst == 0U)) ? 1U : 0U;
}

static uint8_t arm_power_output(uint32_t now_ms)
{
#if GIMBAL_ALLOW_BOARD_POWER_OUTPUT
    if ((s_state != GIMBAL_STATE_SAFE_IDLE) ||
        (s_fault != GIMBAL_FAULT_NONE) ||
        (s_power_output_enabled != 0U))
    {
        return 0U;
    }
    s_power_arm_deadline_ms = now_ms + GIMBAL_POWER_ARM_WINDOW_MS;
    return 1U;
#else
    (void)now_ms;
    return 0U;
#endif
}

static uint8_t power_output_enable(uint32_t now_ms)
{
#if GIMBAL_ALLOW_BOARD_POWER_OUTPUT
    if ((power_arm_is_valid(now_ms) == 0U) ||
        (s_state != GIMBAL_STATE_SAFE_IDLE) ||
        (s_fault != GIMBAL_FAULT_NONE) ||
        (s_power_output_enabled != 0U))
    {
        return 0U;
    }

    HAL_GPIO_WritePin(POWER_24V_1_GPIO_Port,
                      POWER_24V_1_Pin,
                      GPIO_PIN_SET);
    s_power_output_enabled = 1U;
    s_power_arm_deadline_ms = 0U;
    s_power_off_deadline_ms = now_ms + GIMBAL_POWER_ON_TIMEOUT_MS;

    /*
     * Allow the DM4310 to boot, then transmit a disable burst before any
     * OBSERVE or motion request can be accepted.
     */
    s_disable_burst = GIMBAL_DISABLE_BURST_COUNT;
    s_next_motor_tx_ms = now_ms + GIMBAL_POWER_STARTUP_SETTLE_MS;
    return 1U;
#else
    (void)now_ms;
    return 0U;
#endif
}

static float absolute_float(float value)
{
    return (value < 0.0f) ? -value : value;
}

static float clamp_float(float value, float minimum, float maximum)
{
    if (value < minimum)
    {
        return minimum;
    }
    if (value > maximum)
    {
        return maximum;
    }
    return value;
}

static const char *state_name(GimbalState state)
{
    switch (state)
    {
        case GIMBAL_STATE_BOOT: return "BOOT";
        case GIMBAL_STATE_SAFE_IDLE: return "SAFE_IDLE";
        case GIMBAL_STATE_OBSERVE: return "OBSERVE";
        case GIMBAL_STATE_HOLD: return "HOLD";
        case GIMBAL_STATE_STEP: return "STEP";
        case GIMBAL_STATE_SWEEP: return "SWEEP";
        case GIMBAL_STATE_FAULT: return "FAULT";
        case GIMBAL_STATE_COMPETITION_PREP: return "COMP_PREP";
        case GIMBAL_STATE_COMPETITION: return "COMPETITION";
        case GIMBAL_STATE_DELAY_TEST_PREP: return "DELAY_PREP";
        case GIMBAL_STATE_DELAY_TEST: return "DELAY_TEST";
        default: return "UNKNOWN";
    }
}

static void normalize_console_line(char *line)
{
    char *read_cursor;
    char *write_cursor;
    char *end;

    if (line == NULL)
    {
        return;
    }

    read_cursor = line;
    while ((*read_cursor != '\0') &&
           (isspace((unsigned char)*read_cursor) != 0))
    {
        read_cursor++;
    }

    write_cursor = line;
    while (*read_cursor != '\0')
    {
        unsigned char character = (unsigned char)*read_cursor++;
        if (isspace(character) != 0)
        {
            character = (unsigned char)' ';
        }
        else
        {
            character = (unsigned char)toupper(character);
        }
        *write_cursor++ = (char)character;
    }
    *write_cursor = '\0';

    end = write_cursor;
    while ((end > line) && (end[-1] == ' '))
    {
        *--end = '\0';
    }
}

static uint8_t motion_active(void)
{
    return ((s_state == GIMBAL_STATE_HOLD) ||
            (s_state == GIMBAL_STATE_STEP) ||
            (s_state == GIMBAL_STATE_SWEEP)) ? 1U : 0U;
}

static uint8_t vision_sample_valid(uint32_t now_ms)
{
    const uint16_t required =
        VISION_FLAG_BALL_VALID |
        VISION_FLAG_PIPE_VALID |
        VISION_FLAG_CALIBRATED;
    return ((s_vision_received != 0U) &&
            ((s_vision.flags & required) == required) &&
            ((uint32_t)(now_ms - s_vision.receive_time_ms) <=
             GIMBAL_VISION_STALE_MS) &&
            (s_vision.confidence_permille >= 300U)) ? 1U : 0U;
}

static void competition_q2_stall_reset(void)
{
    s_q2_stall_start_ms = 0U;
    s_q2_stall_last_sample_ms = 0U;
    s_q2_stall_min_um = 0;
    s_q2_stall_max_um = 0;
}

static void competition_velocity_reset(void)
{
    s_velocity_history_count = 0U;
    s_velocity_history_head = 0U;
    s_velocity_last_sample_ms = 0U;
    s_velocity_window_ms = 0U;
    s_velocity_filtered_um_s = 0;
}

static void competition_q2_motion_reset(void)
{
    s_q2_motion_last_sample_ms = 0U;
    s_q2_motion_window_start_ms = 0U;
    s_q2_motion_min_um = 0;
    s_q2_motion_max_um = 0;
    s_q2_stationary_ms = 0U;
}

static void enter_stopped_state(GimbalFault fault, uint32_t now_ms)
{
    power_output_force_off();
    s_fault = fault;
    s_state = (fault == GIMBAL_FAULT_NONE) ?
              GIMBAL_STATE_SAFE_IDLE : GIMBAL_STATE_FAULT;
    s_state_start_ms = now_ms;
    s_arm_deadline_ms = 0U;
    s_disable_burst = GIMBAL_DISABLE_BURST_COUNT;
    s_next_motor_tx_ms = now_ms;
    g_gimbal_debug.arm_key = 0U;
    s_zero_referenced_motion = 0U;
    s_command_pipe_angle_mdeg = 0;
    s_q2_breakaway_deadline_ms = 0U;
    s_q2_final_capture_latched = 0U;
    competition_q2_stall_reset();
    if (s_competition_run_id != 0U)
    {
        s_competition_phase =
            fault == GIMBAL_FAULT_NONE ?
            COMPETITION_IDLE : COMPETITION_FAULT;
        s_command_pipe_angle_mdeg = 0;
    }
}

static uint8_t feedback_ready(uint32_t now_ms)
{
    const Dm4310Status *motor = dm4310_status();
    return ((motor->online != 0U) &&
            ((uint32_t)(now_ms - motor->last_rx_ms) <=
             GIMBAL_FEEDBACK_TIMEOUT_MS)) ? 1U : 0U;
}

static uint8_t arm_is_valid(uint32_t now_ms)
{
    if (s_arm_deadline_ms == 0U)
    {
        return 0U;
    }
    return (time_reached(now_ms, s_arm_deadline_ms) == 0U) ? 1U : 0U;
}

static void arm_for_motion(uint32_t now_ms)
{
    s_arm_deadline_ms = now_ms + GIMBAL_ARM_WINDOW_MS;
}

static float active_motion_maximum_rad(void)
{
    float maximum_angle_deg =
        (s_calrun_phase != CALRUN_IDLE) ?
        GIMBAL_CALRUN_MAX_ANGLE_DEG :
        GIMBAL_DESKTOP_MAX_ANGLE_DEG;

    return maximum_angle_deg * DEG_TO_RAD;
}

static uint8_t start_motion(GimbalState requested_state,
                            int32_t tenths_degree,
                            uint8_t use_level_zero,
                            uint32_t now_ms)
{
    const Dm4310Status *motor = dm4310_status();
    float requested_offset_rad;
    float maximum_rad = active_motion_maximum_rad();

    if ((power_output_is_ready() == 0U) ||
        (arm_is_valid(now_ms) == 0U) ||
        (feedback_ready(now_ms) == 0U) ||
        ((use_level_zero != 0U) && (s_level_zero_valid == 0U)))
    {
        return 0U;
    }

    requested_offset_rad =
        ((float)tenths_degree * 0.1f) * DEG_TO_RAD;
    requested_offset_rad =
        clamp_float(requested_offset_rad, -maximum_rad, maximum_rad);

    s_neutral_rad = (use_level_zero != 0U) ?
                    s_level_zero_rad : motor->position_rad;
    s_command_rad = motor->position_rad;
    s_sweep_amplitude_rad = absolute_float(requested_offset_rad);
    s_sweep_direction = 1;

    if (requested_state == GIMBAL_STATE_HOLD)
    {
        s_requested_rad = s_neutral_rad;
    }
    else if (requested_state == GIMBAL_STATE_STEP)
    {
        s_requested_rad = s_neutral_rad + requested_offset_rad;
    }
    else
    {
        if (s_sweep_amplitude_rad < (0.1f * DEG_TO_RAD))
        {
            s_sweep_amplitude_rad = 0.1f * DEG_TO_RAD;
        }
        s_requested_rad = s_neutral_rad + s_sweep_amplitude_rad;
    }

    if (dm4310_enable() != HAL_OK)
    {
        enter_stopped_state(GIMBAL_FAULT_CAN_TX, now_ms);
        return 0U;
    }

    s_state = requested_state;
    s_zero_referenced_motion = use_level_zero;
    s_post_jog_disable_deadline_ms = 0U;
    s_fault = GIMBAL_FAULT_NONE;
    s_state_start_ms = now_ms;
    s_next_motor_tx_ms = now_ms;
    s_arm_deadline_ms = 0U;
    return 1U;
}

static uint8_t capture_level_zero(uint32_t now_ms)
{
    const Dm4310Status *motor = dm4310_status();

    if ((s_state != GIMBAL_STATE_OBSERVE) ||
        (feedback_ready(now_ms) == 0U) ||
        (motor->error != 0U))
    {
        return 0U;
    }

    s_level_zero_rad = motor->position_rad;
    s_level_zero_valid = 1U;
    s_neutral_rad = s_level_zero_rad;
    s_requested_rad = s_level_zero_rad;
    s_command_rad = s_level_zero_rad;
    s_arm_deadline_ms = 0U;
    return 1U;
}

static void process_request(GimbalRequest request,
                            int32_t tenths_degree,
                            uint8_t debugger_arm,
                            uint32_t now_ms)
{
    if (debugger_arm != 0U)
    {
        arm_for_motion(now_ms);
    }

    switch (request)
    {
        case GIMBAL_REQUEST_NONE:
            break;

        case GIMBAL_REQUEST_STOP:
            enter_stopped_state(GIMBAL_FAULT_NONE, now_ms);
            break;

        case GIMBAL_REQUEST_OBSERVE:
            if ((motion_active() == 0U) &&
                (power_output_is_ready() != 0U))
            {
                s_state = GIMBAL_STATE_OBSERVE;
                s_fault = GIMBAL_FAULT_NONE;
                s_state_start_ms = now_ms;
                s_next_motor_tx_ms = now_ms;
                s_arm_deadline_ms = 0U;
            }
            break;

        case GIMBAL_REQUEST_HOLD:
            (void)start_motion(GIMBAL_STATE_HOLD, 0, 0U, now_ms);
            break;

        case GIMBAL_REQUEST_STEP:
            (void)start_motion(GIMBAL_STATE_STEP,
                               tenths_degree,
                               0U,
                               now_ms);
            break;

        case GIMBAL_REQUEST_SWEEP:
            (void)start_motion(GIMBAL_STATE_SWEEP,
                               tenths_degree,
                               0U,
                               now_ms);
            break;

        case GIMBAL_REQUEST_CLEAR_FAULT:
            if (motion_active() == 0U)
            {
                enter_stopped_state(GIMBAL_FAULT_NONE, now_ms);
            }
            break;

        default:
            if (motion_active() == 0U)
            {
                s_fault = GIMBAL_FAULT_BAD_COMMAND;
            }
            break;
    }
}

static int32_t parse_tenths_degree(const char *text)
{
    double degrees;
    char *end;

    if (text == NULL)
    {
        return 0;
    }

    degrees = strtod(text, &end);
    if (end == text)
    {
        return 0;
    }
    if (degrees > GIMBAL_DESKTOP_MAX_ANGLE_DEG)
    {
        degrees = GIMBAL_DESKTOP_MAX_ANGLE_DEG;
    }
    if (degrees < -GIMBAL_DESKTOP_MAX_ANGLE_DEG)
    {
        degrees = -GIMBAL_DESKTOP_MAX_ANGLE_DEG;
    }
    return (int32_t)(degrees * 10.0);
}

#if GIMBAL_ENABLE_MANUAL_CALRUN
static uint8_t parse_calrun_argument(const char *text,
                                     int32_t *tenths_degree)
{
    const char *cursor;
    char *end;
    double degrees;

    if ((text == NULL) || (tenths_degree == NULL) ||
        (strncmp(text, "4310", 4U) != 0))
    {
        return 0U;
    }

    cursor = text + 4;
    if (*cursor != ' ')
    {
        return 0U;
    }
    while (*cursor == ' ')
    {
        cursor++;
    }

    degrees = strtod(cursor, &end);
    if ((end == cursor) || (*end != '\0') ||
        (degrees < 0.1) ||
        (degrees > (double)GIMBAL_CALRUN_MAX_ANGLE_DEG))
    {
        return 0U;
    }

    *tenths_degree = (int32_t)((degrees * 10.0) + 0.5);
    return 1U;
}

static uint8_t calrun_begin(int32_t amplitude_tenths_degree,
                            uint32_t now_ms)
{
    if ((s_calrun_phase != CALRUN_IDLE) ||
        (s_state != GIMBAL_STATE_SAFE_IDLE) ||
        (s_fault != GIMBAL_FAULT_NONE) ||
        (s_power_output_enabled != 0U))
    {
        return 0U;
    }

    if ((arm_power_output(now_ms) == 0U) ||
        (power_output_enable(now_ms) == 0U))
    {
        enter_stopped_state(GIMBAL_FAULT_BAD_COMMAND, now_ms);
        return 0U;
    }

    s_calrun_amplitude_tenths_deg = amplitude_tenths_degree;
    s_calrun_phase = CALRUN_WAIT_READY;
    s_calrun_deadline_ms = now_ms + GIMBAL_CALRUN_PREP_TIMEOUT_MS;
    return 1U;
}
#endif

static uint8_t calrun_start_leg(int32_t tenths_degree,
                                CalRunPhase phase,
                                uint32_t now_ms)
{
    const Dm4310Status *motor = dm4310_status();

    arm_for_motion(now_ms);
    if (start_motion(GIMBAL_STATE_STEP,
                     tenths_degree,
                     1U,
                     now_ms) == 0U)
    {
        return 0U;
    }

    s_calrun_phase = phase;
    (void)usb_console_printf(
        "CALRUN LEG=%lu FROM=%ldmrad Z=%ldmrad TARGET=%ldmrad\r\n",
        (unsigned long)phase,
        (long)(motor->position_rad * 1000.0f),
        (long)(s_level_zero_rad * 1000.0f),
        (long)(s_requested_rad * 1000.0f));
    return 1U;
}

static uint8_t calrun_leg_is_finished(uint32_t now_ms)
{
    const Dm4310Status *motor = dm4310_status();

    return ((s_state == GIMBAL_STATE_OBSERVE) &&
            (s_zero_referenced_motion == 0U) &&
            (s_post_jog_disable_deadline_ms == 0U) &&
            (feedback_ready(now_ms) != 0U) &&
            (motor->error == 0U)) ? 1U : 0U;
}

static void run_calrun(uint32_t now_ms)
{
    const Dm4310Status *motor = dm4310_status();

    switch (s_calrun_phase)
    {
        case CALRUN_IDLE:
            break;

        case CALRUN_WAIT_READY:
            if (time_reached(now_ms, s_calrun_deadline_ms) != 0U)
            {
                (void)usb_console_write(
                    "CALRUN ABORT: no motor feedback in 15 s; "
                    "OUT1 OFF, SAFE_IDLE\r\n");
                enter_stopped_state(GIMBAL_FAULT_NONE, now_ms);
                break;
            }

            if (power_output_is_ready() == 0U)
            {
                break;
            }

            /*
             * The initial five-frame burst can finish before a cold DM4310
             * is ready to answer. Keep sending the same torque-disable frame
             * used by OBSERVE so startup can never deadlock waiting for a
             * reply while the drive remains explicitly disabled.
             */
            if (time_reached(now_ms, s_next_motor_tx_ms) != 0U)
            {
                if (dm4310_query() != HAL_OK)
                {
                    (void)usb_console_write(
                        "CALRUN ABORT: CAN transmit failed\r\n");
                    enter_stopped_state(GIMBAL_FAULT_CAN_TX, now_ms);
                    break;
                }
                s_next_motor_tx_ms =
                    now_ms + GIMBAL_OBSERVE_QUERY_PERIOD_MS;
            }

            if ((feedback_ready(now_ms) != 0U) &&
                (motor->error != 0U) &&
                (motor->error != 1U))
            {
                (void)usb_console_printf(
                    "CALRUN ABORT: motor ERR=0x%lX\r\n",
                    (unsigned long)motor->error);
                enter_stopped_state(GIMBAL_FAULT_MOTOR_REPORTED, now_ms);
                break;
            }

            if ((feedback_ready(now_ms) != 0U) && (motor->error == 0U))
            {
                s_state = GIMBAL_STATE_OBSERVE;
                s_state_start_ms = now_ms;
                s_next_motor_tx_ms = now_ms;
                s_calrun_phase = CALRUN_COUNTDOWN;
                s_calrun_deadline_ms =
                    now_ms + GIMBAL_CALRUN_COUNTDOWN_MS;
                (void)usb_console_write(
                    "CALRUN READY: motion starts in 3 s; send STOP to abort\r\n");
            }
            break;

        case CALRUN_COUNTDOWN:
            if (time_reached(now_ms, s_calrun_deadline_ms) == 0U)
            {
                break;
            }
            if ((capture_level_zero(now_ms) == 0U) ||
                (calrun_start_leg(s_calrun_amplitude_tenths_deg,
                                  CALRUN_POSITIVE,
                                  now_ms) == 0U))
            {
                (void)usb_console_write(
                    "CALRUN ABORT: zero/positive start failed\r\n");
                enter_stopped_state(GIMBAL_FAULT_FEEDBACK_TIMEOUT, now_ms);
            }
            break;

        case CALRUN_POSITIVE:
            if ((calrun_leg_is_finished(now_ms) != 0U) &&
                (calrun_start_leg(0,
                                  CALRUN_RETURN_FROM_POSITIVE,
                                  now_ms) == 0U))
            {
                (void)usb_console_write(
                    "CALRUN ABORT: first return failed\r\n");
                enter_stopped_state(GIMBAL_FAULT_FEEDBACK_TIMEOUT, now_ms);
            }
            break;

        case CALRUN_RETURN_FROM_POSITIVE:
            if ((calrun_leg_is_finished(now_ms) != 0U) &&
                (calrun_start_leg(-s_calrun_amplitude_tenths_deg,
                                  CALRUN_NEGATIVE,
                                  now_ms) == 0U))
            {
                (void)usb_console_write(
                    "CALRUN ABORT: negative start failed\r\n");
                enter_stopped_state(GIMBAL_FAULT_FEEDBACK_TIMEOUT, now_ms);
            }
            break;

        case CALRUN_NEGATIVE:
            if ((calrun_leg_is_finished(now_ms) != 0U) &&
                (calrun_start_leg(0,
                                  CALRUN_FINAL_RETURN,
                                  now_ms) == 0U))
            {
                (void)usb_console_write(
                    "CALRUN ABORT: final return failed\r\n");
                enter_stopped_state(GIMBAL_FAULT_FEEDBACK_TIMEOUT, now_ms);
            }
            break;

        case CALRUN_FINAL_RETURN:
            if (calrun_leg_is_finished(now_ms) != 0U)
            {
                (void)usb_console_printf(
                    "CALRUN COMPLETE P=%ldmrad OFF=%ldmrad; OUT1 OFF\r\n",
                    (long)(motor->position_rad * 1000.0f),
                    (long)((motor->position_rad - s_level_zero_rad) *
                           1000.0f));
                enter_stopped_state(GIMBAL_FAULT_NONE, now_ms);
            }
            break;

        default:
            (void)usb_console_write("CALRUN ABORT: invalid phase\r\n");
            enter_stopped_state(GIMBAL_FAULT_BAD_COMMAND, now_ms);
            break;
    }
}

static void send_status(uint32_t now_ms)
{
    const Dm4310Status *motor = dm4310_status();
    int32_t position_mrad = (int32_t)(motor->position_rad * 1000.0f);
    int32_t velocity_mrad_s =
        (int32_t)(motor->velocity_rad_s * 1000.0f);
    int32_t target_mrad = (int32_t)(s_command_rad * 1000.0f);
    int32_t zero_mrad = (int32_t)(s_level_zero_rad * 1000.0f);
    int32_t offset_mrad =
        (s_level_zero_valid != 0U) ?
        (int32_t)((motor->position_rad - s_level_zero_rad) * 1000.0f) : 0;
    uint32_t age = (motor->rx_count == 0U) ?
                   0xFFFFFFFFUL : (uint32_t)(now_ms - motor->last_rx_ms);
    uint32_t power_arm_remaining =
        (power_arm_is_valid(now_ms) != 0U) ?
        (uint32_t)(s_power_arm_deadline_ms - now_ms) : 0U;
    uint32_t power_on_remaining =
        ((s_power_output_enabled != 0U) &&
         (time_reached(now_ms, s_power_off_deadline_ms) == 0U)) ?
        (uint32_t)(s_power_off_deadline_ms - now_ms) : 0U;

    (void)usb_console_printf(
        "STATE=%s FAULT=%lu ONLINE=%u ERR=0x%lX "
        "P=%ldmrad V=%ldmrad/s CMD=%ldmrad AGE=%lums "
        "RX=%lu TX=%lu TXERR=%lu PWR=%u PWRARM=%lums PWRLEFT=%lums "
        "CAL=%lu ZVALID=%u Z=%ldmrad OFF=%ldmrad\r\n",
        state_name(s_state),
        (unsigned long)s_fault,
        (unsigned int)motor->online,
        (unsigned long)motor->error,
        (long)position_mrad,
        (long)velocity_mrad_s,
        (long)target_mrad,
        (unsigned long)age,
        (unsigned long)motor->rx_count,
        (unsigned long)motor->tx_count,
        (unsigned long)motor->tx_error_count,
        (unsigned int)s_power_output_enabled,
        (unsigned long)power_arm_remaining,
        (unsigned long)power_on_remaining,
        (unsigned long)s_calrun_phase,
        (unsigned int)s_level_zero_valid,
        (long)zero_mrad,
        (long)offset_mrad);
}

static void process_console(uint32_t now_ms)
{
    char line[96];
    char *argument;

    while (usb_console_get_line(line, sizeof(line)) != 0U)
    {
        normalize_console_line(line);

        argument = strchr(line, ' ');
        if (argument != NULL)
        {
            *argument++ = '\0';
            while (*argument == ' ')
            {
                argument++;
            }
        }

        if ((s_calrun_phase != CALRUN_IDLE) &&
            (strcmp(line, "PING") != 0) &&
            (strcmp(line, "HELP") != 0) &&
            (strcmp(line, "STATUS") != 0) &&
            (strcmp(line, "STOP") != 0))
        {
            (void)usb_console_write(
                "DENIED: CALRUN active; only STATUS or STOP allowed\r\n");
            continue;
        }
#if GIMBAL_ENABLE_DELAY_TEST
        if ((s_delay_test_phase >= DELAY_TEST_WAIT_POWER) &&
            (s_delay_test_phase <= DELAY_TEST_READY_MEASURE) &&
            (strcmp(line, "PING") != 0) &&
            (strcmp(line, "HELP") != 0) &&
            (strcmp(line, "STATUS") != 0) &&
            (strcmp(line, "STOP") != 0))
        {
            (void)usb_console_write(
                "DENIED: DelayTest owns motion; "
                "only STATUS or STOP allowed\r\n");
            continue;
        }
#endif

        if (strcmp(line, "PING") == 0)
        {
            (void)usb_console_printf("PONG BUILD=%lu DESKTOP=%u\r\n",
                                     (unsigned long)GIMBAL_BUILD_ID,
                                     (unsigned int)GIMBAL_DESKTOP_BUILD);
        }
        else if (strcmp(line, "HELP") == 0)
        {
            (void)usb_console_write(
                "PING | STATUS | OBSERVE | ARM | HOLD | "
                "STEP deg | SWEEP deg | ZERO | JOG deg | STOP | CLEAR\r\n"
                "PWRARM 4310 | PWRON | PWROFF | CALRUN 4310 deg\r\n"
                "ZERO requires OBSERVE; JOG requires ZERO then ARM.\r\n"
                "CALRUN is manual-only: auto power/+deg/0/-deg/0/power-off.\r\n"
                "OUT1 expires in 180 s; STOP/fault/reset turns it off.\r\n");
        }
        else if (strcmp(line, "STATUS") == 0)
        {
            send_status(now_ms);
        }
        else if (strcmp(line, "CALRUN") == 0)
        {
            int32_t amplitude_tenths_degree;
#if GIMBAL_ENABLE_MANUAL_CALRUN
            if ((parse_calrun_argument(argument,
                                       &amplitude_tenths_degree) != 0U) &&
                (calrun_begin(amplitude_tenths_degree, now_ms) != 0U))
            {
                (void)usb_console_printf(
                    "CALRUN START amplitude=%ld.%lddeg; "
                    "OUT1 ON, keep clear\r\n",
                    (long)(amplitude_tenths_degree / 10),
                    (long)(amplitude_tenths_degree % 10));
            }
            else
            {
                (void)usb_console_write(
                    "DENIED: use CALRUN 4310 deg (0.1..45) from safe idle\r\n");
            }
#else
            (void)amplitude_tenths_degree;
            (void)usb_console_write(
                "DENIED: CALRUN disabled in this build\r\n");
#endif
        }
        else if (strcmp(line, "OBSERVE") == 0)
        {
            if (power_output_is_ready() != 0U)
            {
                process_request(GIMBAL_REQUEST_OBSERVE, 0, 0U, now_ms);
                (void)usb_console_write(
                    "OK OBSERVE (motor torque not enabled)\r\n");
            }
            else
            {
                (void)usb_console_write(
                    "DENIED: OUT1 off or startup disable burst pending\r\n");
            }
        }
        else if (strcmp(line, "PWRARM") == 0)
        {
            if ((argument != NULL) &&
                (strcmp(argument, "4310") == 0) &&
                (arm_power_output(now_ms) != 0U))
            {
                (void)usb_console_write(
                    "OK OUT1 ARMED FOR 3000 ms; send PWRON\r\n");
            }
            else
            {
                (void)usb_console_write(
                    "DENIED: use PWRARM 4310 from safe idle\r\n");
            }
        }
        else if (strcmp(line, "PWRON") == 0)
        {
            if (power_output_enable(now_ms) != 0U)
            {
                (void)usb_console_write(
                    "OK OUT1 ON FOR 300000 ms; wait before OBSERVE\r\n");
            }
            else
            {
                (void)usb_console_write(
                    "DENIED: PWRARM 4310 required within 3 s\r\n");
            }
        }
        else if (strcmp(line, "PWROFF") == 0)
        {
            enter_stopped_state(GIMBAL_FAULT_NONE, now_ms);
            (void)usb_console_write("OK OUT1 OFF; MOTOR UNPOWERED\r\n");
        }
        else if (strcmp(line, "ARM") == 0)
        {
            if (feedback_ready(now_ms) != 0U)
            {
                arm_for_motion(now_ms);
                (void)usb_console_write("OK ARMED FOR 3000 ms\r\n");
            }
            else
            {
                (void)usb_console_write(
                    "DENIED: no fresh motor feedback; run OBSERVE first\r\n");
            }
        }
        else if (strcmp(line, "HOLD") == 0)
        {
            if (start_motion(GIMBAL_STATE_HOLD, 0, 0U, now_ms) != 0U)
            {
                (void)usb_console_write("OK HOLD (auto-stop 15 s)\r\n");
            }
            else
            {
                (void)usb_console_write(
                    "DENIED: ARM and fresh feedback required\r\n");
            }
        }
        else if (strcmp(line, "STEP") == 0)
        {
            int32_t value = parse_tenths_degree(argument);
            if (start_motion(GIMBAL_STATE_STEP, value, 0U, now_ms) != 0U)
            {
                (void)usb_console_write("OK STEP (auto-stop 3 s)\r\n");
            }
            else
            {
                (void)usb_console_write(
                    "DENIED: ARM and fresh feedback required\r\n");
            }
        }
        else if (strcmp(line, "SWEEP") == 0)
        {
            int32_t value = parse_tenths_degree(argument);
            if (value < 0)
            {
                value = -value;
            }
            if (start_motion(GIMBAL_STATE_SWEEP, value, 0U, now_ms) != 0U)
            {
                (void)usb_console_write("OK SWEEP (auto-stop 6 s)\r\n");
            }
            else
            {
                (void)usb_console_write(
                    "DENIED: ARM and fresh feedback required\r\n");
            }
        }
        else if (strcmp(line, "ZERO") == 0)
        {
            if (capture_level_zero(now_ms) != 0U)
            {
                (void)usb_console_printf(
                    "OK LEVEL ZERO P=%ldmrad; send ARM then JOG deg\r\n",
                    (long)(s_level_zero_rad * 1000.0f));
            }
            else
            {
                (void)usb_console_write(
                    "DENIED: ZERO requires OBSERVE and fresh feedback\r\n");
            }
        }
        else if (strcmp(line, "JOG") == 0)
        {
            int32_t value = parse_tenths_degree(argument);
            if ((argument != NULL) &&
                (start_motion(GIMBAL_STATE_STEP,
                              value,
                              1U,
                              now_ms) != 0U))
            {
                const Dm4310Status *motor = dm4310_status();
                (void)usb_console_printf(
                    "OK JOG FROM=%ldmrad Z=%ldmrad TARGET=%ldmrad "
                    "(3 s, then disabled OBSERVE)\r\n",
                    (long)(motor->position_rad * 1000.0f),
                    (long)(s_level_zero_rad * 1000.0f),
                    (long)(s_requested_rad * 1000.0f));
            }
            else
            {
                (void)usb_console_write(
                    "DENIED: ZERO, ARM and fresh feedback required\r\n");
            }
        }
        else if (strcmp(line, "STOP") == 0)
        {
            enter_stopped_state(GIMBAL_FAULT_NONE, now_ms);
            (void)usb_console_write("OK STOPPED/DISABLED; OUT1 OFF\r\n");
        }
        else if (strcmp(line, "CLEAR") == 0)
        {
            process_request(GIMBAL_REQUEST_CLEAR_FAULT, 0, 0U, now_ms);
            (void)usb_console_write("OK FAULT CLEARED; MOTOR DISABLED\r\n");
        }
        else
        {
            (void)usb_console_write("ERR unknown command; send HELP\r\n");
        }
    }
}

static void run_safe_transmit(uint32_t now_ms)
{
    if ((s_disable_burst > 0U) &&
        (time_reached(now_ms, s_next_motor_tx_ms) != 0U))
    {
        if (dm4310_disable() == HAL_OK)
        {
            s_disable_burst--;
        }
        else if (s_power_output_enabled != 0U)
        {
            enter_stopped_state(GIMBAL_FAULT_CAN_TX, now_ms);
            return;
        }
        else
        {
            s_disable_burst--;
        }
        s_next_motor_tx_ms = now_ms + GIMBAL_MOTOR_COMMAND_PERIOD_MS;
    }
}

#if GIMBAL_ENABLE_DELAY_TEST
static void delay_test_stability_reset(void)
{
    s_delay_test_stable_start_ms = 0U;
    s_delay_test_stable_last_sample_ms = 0U;
    s_delay_test_stable_min_um = 0;
    s_delay_test_stable_max_um = 0;
    s_delay_test_stable_sample_count = 0U;
    s_delay_test_stable_ready = 0U;
}

static void delay_test_stability_begin(const VisionSample *sample)
{
    s_delay_test_stable_start_ms = sample->receive_time_ms;
    s_delay_test_stable_last_sample_ms = sample->receive_time_ms;
    s_delay_test_stable_min_um = sample->position_um;
    s_delay_test_stable_max_um = sample->position_um;
    s_delay_test_stable_sample_count = 1U;
    s_delay_test_stable_ready = 0U;
}

static void delay_test_stability_on_sample(const VisionSample *sample)
{
    const uint16_t required =
        VISION_FLAG_BALL_VALID |
        VISION_FLAG_PIPE_VALID |
        VISION_FLAG_CALIBRATED;
    int32_t position_um = sample->position_um;
    uint32_t gap_ms;
    int32_t span_um;

    if (s_delay_test_phase != DELAY_TEST_READY_MEASURE)
    {
        delay_test_stability_reset();
        return;
    }
    if (position_um < 0)
    {
        position_um = -position_um;
    }
    if (((sample->flags & required) != required) ||
        ((sample->flags &
          (VISION_FLAG_OCCLUDED | VISION_FLAG_OUT_OF_RANGE)) != 0U) ||
        (sample->confidence_permille < 300U) ||
        (position_um > GIMBAL_DELAY_TEST_START_POS_UM))
    {
        delay_test_stability_reset();
        return;
    }

    gap_ms = sample->receive_time_ms -
             s_delay_test_stable_last_sample_ms;
    if ((s_delay_test_stable_start_ms == 0U) ||
        (s_delay_test_stable_last_sample_ms == 0U) ||
        (gap_ms > GIMBAL_VISION_STALE_MS))
    {
        delay_test_stability_begin(sample);
        return;
    }

    s_delay_test_stable_last_sample_ms = sample->receive_time_ms;
    if (sample->position_um < s_delay_test_stable_min_um)
    {
        s_delay_test_stable_min_um = sample->position_um;
    }
    if (sample->position_um > s_delay_test_stable_max_um)
    {
        s_delay_test_stable_max_um = sample->position_um;
    }
    s_delay_test_stable_sample_count++;
    span_um =
        s_delay_test_stable_max_um - s_delay_test_stable_min_um;
    if (span_um > GIMBAL_DELAY_TEST_STABLE_SPAN_UM)
    {
        delay_test_stability_begin(sample);
        return;
    }
    if (((uint32_t)(sample->receive_time_ms -
                    s_delay_test_stable_start_ms) >=
         GIMBAL_DELAY_TEST_STABLE_WINDOW_MS) &&
        (s_delay_test_stable_sample_count >=
         GIMBAL_DELAY_TEST_STABLE_SAMPLES))
    {
        s_delay_test_stable_ready = 1U;
    }
}

static uint8_t delay_test_mode_owns_app(void)
{
    return ((s_delay_test_phase >= DELAY_TEST_WAIT_POWER) &&
            (s_delay_test_phase <= DELAY_TEST_READY_MEASURE)) ? 1U : 0U;
}

static uint8_t delay_test_arm_is_valid(uint32_t now_ms)
{
    return ((s_delay_test_arm_deadline_ms != 0U) &&
            (time_reached(now_ms,
                          s_delay_test_arm_deadline_ms) == 0U)) ? 1U : 0U;
}

static void delay_test_deny(uint32_t request)
{
    s_delay_test_denied_count++;
    s_delay_test_last_denied_request = request;
}

static void delay_test_fail(GimbalFault fault, uint32_t now_ms)
{
    enter_stopped_state(fault, now_ms);
    s_delay_test_phase = DELAY_TEST_FAULT;
}

static void delay_test_start_pulse(
    uint8_t measurement, uint32_t now_ms)
{
    float pipe_rad =
        ((float)GIMBAL_DELAY_TEST_PIPE_ANGLE_MDEG / 1000.0f) *
        DEG_TO_RAD;

    s_requested_rad =
        s_level_zero_rad -
        (pipe_rad / GIMBAL_PIPE_PER_MOTOR_RATIO);
    s_command_rad = s_requested_rad;
    s_command_pipe_angle_mdeg =
        GIMBAL_DELAY_TEST_PIPE_ANGLE_MDEG;
    s_delay_test_measurement_active = measurement;
    s_delay_test_return_settle_start_ms = 0U;
    s_delay_test_last_pulse_start_ms = now_ms;
    delay_test_stability_reset();
    s_delay_test_deadline_ms =
        now_ms + GIMBAL_DELAY_TEST_HOLD_MS;
    s_delay_test_phase = DELAY_TEST_PULSE;
    s_next_motor_tx_ms = now_ms;
}

static void delay_test_process_debug_request(uint32_t now_ms)
{
    uint32_t request = g_gimbal_debug.delay_test_request;
    uint8_t key_present =
        (g_gimbal_debug.delay_test_key ==
         GIMBAL_DELAY_TEST_ARM_KEY) ? 1U : 0U;

    if (request == GIMBAL_DELAY_REQUEST_NONE)
    {
        if (key_present != 0U)
        {
            s_delay_test_arm_deadline_ms =
                now_ms + GIMBAL_DELAY_TEST_ARM_WINDOW_MS;
        }
        g_gimbal_debug.delay_test_key = 0U;
        return;
    }

    g_gimbal_debug.delay_test_request =
        GIMBAL_DELAY_REQUEST_NONE;
    g_gimbal_debug.delay_test_key = 0U;
    if (request == GIMBAL_DELAY_REQUEST_STOP)
    {
        enter_stopped_state(GIMBAL_FAULT_NONE, now_ms);
        return;
    }

    if (key_present != 0U)
    {
        s_delay_test_arm_deadline_ms =
            now_ms + GIMBAL_DELAY_TEST_ARM_WINDOW_MS;
    }
    if (delay_test_arm_is_valid(now_ms) == 0U)
    {
        delay_test_deny(request);
        return;
    }
    s_delay_test_arm_deadline_ms = 0U;

    switch ((GimbalDelayTestRequest)request)
    {
        case GIMBAL_DELAY_REQUEST_PREPARE:
            if ((s_delay_test_phase != DELAY_TEST_IDLE) ||
                (s_state != GIMBAL_STATE_SAFE_IDLE) ||
                (s_fault != GIMBAL_FAULT_NONE) ||
                (s_competition_run_id != 0U) ||
                (s_calrun_phase != CALRUN_IDLE) ||
                (arm_power_output(now_ms) == 0U) ||
                (power_output_enable(now_ms) == 0U))
            {
                delay_test_deny(request);
                return;
            }
            s_delay_test_validated = 0U;
            s_delay_test_mechanical_ack = 0U;
            s_delay_test_measurement_active = 0U;
            delay_test_stability_reset();
            s_delay_test_phase = DELAY_TEST_WAIT_POWER;
            s_delay_test_deadline_ms =
                now_ms + GIMBAL_DELAY_TEST_PREP_TIMEOUT_MS;
            s_state = GIMBAL_STATE_DELAY_TEST_PREP;
            s_state_start_ms = now_ms;
            break;

        case GIMBAL_DELAY_REQUEST_VALIDATE_EMPTY:
            if (s_delay_test_phase != DELAY_TEST_READY_VALIDATE)
            {
                delay_test_deny(request);
                return;
            }
            s_power_off_deadline_ms =
                now_ms + GIMBAL_POWER_ON_TIMEOUT_MS;
            delay_test_start_pulse(0U, now_ms);
            break;

        case GIMBAL_DELAY_REQUEST_ACK_MECHANICAL:
            if ((s_delay_test_phase != DELAY_TEST_WAIT_ACK) ||
                (s_delay_test_validated == 0U))
            {
                delay_test_deny(request);
                return;
            }
            s_delay_test_mechanical_ack = 1U;
            s_delay_test_phase = DELAY_TEST_READY_MEASURE;
            delay_test_stability_reset();
            s_power_off_deadline_ms =
                now_ms + GIMBAL_POWER_ON_TIMEOUT_MS;
            break;

        case GIMBAL_DELAY_REQUEST_MEASURE:
        {
            int32_t position_um = s_vision.position_um;
            if (position_um < 0)
            {
                position_um = -position_um;
            }
            if ((s_delay_test_phase != DELAY_TEST_READY_MEASURE) ||
                (s_delay_test_mechanical_ack == 0U) ||
                (jetson_link_online() == 0U) ||
                (jetson_link_ready() == 0U) ||
                (vision_sample_valid(now_ms) == 0U) ||
                (position_um > GIMBAL_DELAY_TEST_START_POS_UM) ||
                (s_delay_test_stable_ready == 0U))
            {
                delay_test_deny(request);
                return;
            }
            s_power_off_deadline_ms =
                now_ms + GIMBAL_POWER_ON_TIMEOUT_MS;
            delay_test_start_pulse(1U, now_ms);
            break;
        }

        default:
            delay_test_deny(request);
            break;
    }
}

static void run_delay_test(uint32_t now_ms)
{
    const Dm4310Status *motor = dm4310_status();
    float desired_motor_rad = s_level_zero_rad;

    if ((s_delay_test_phase == DELAY_TEST_READY_MEASURE) &&
        (s_delay_test_stable_last_sample_ms != 0U) &&
        ((uint32_t)(now_ms - s_delay_test_stable_last_sample_ms) >
         GIMBAL_VISION_STALE_MS))
    {
        delay_test_stability_reset();
    }

    if (s_delay_test_phase == DELAY_TEST_WAIT_POWER)
    {
        run_safe_transmit(now_ms);
        if (time_reached(
                now_ms, s_delay_test_deadline_ms) != 0U)
        {
            delay_test_fail(
                GIMBAL_FAULT_FEEDBACK_TIMEOUT, now_ms);
            return;
        }
        if (power_output_is_ready() == 0U)
        {
            return;
        }
        if (feedback_ready(now_ms) == 0U)
        {
            if (time_reached(now_ms, s_next_motor_tx_ms) != 0U)
            {
                (void)dm4310_query();
                s_next_motor_tx_ms =
                    now_ms + GIMBAL_OBSERVE_QUERY_PERIOD_MS;
            }
            return;
        }
        if (motor->error != 0U)
        {
            delay_test_fail(
                GIMBAL_FAULT_MOTOR_REPORTED, now_ms);
            return;
        }

        s_level_zero_rad = motor->position_rad;
        s_level_zero_valid = 1U;
        s_neutral_rad = s_level_zero_rad;
        s_requested_rad = s_level_zero_rad;
        s_command_rad = s_level_zero_rad;
        if (dm4310_enable() != HAL_OK)
        {
            delay_test_fail(GIMBAL_FAULT_CAN_TX, now_ms);
            return;
        }
        s_delay_test_phase = DELAY_TEST_COUNTDOWN;
        s_delay_test_deadline_ms =
            now_ms + GIMBAL_DELAY_TEST_COUNTDOWN_MS;
        s_state = GIMBAL_STATE_DELAY_TEST;
        s_state_start_ms = now_ms;
        s_next_motor_tx_ms = now_ms;
    }

    if (feedback_ready(now_ms) == 0U)
    {
        delay_test_fail(
            GIMBAL_FAULT_FEEDBACK_TIMEOUT, now_ms);
        return;
    }
    if ((motor->error != 0U) && (motor->error != 1U))
    {
        delay_test_fail(
            GIMBAL_FAULT_MOTOR_REPORTED, now_ms);
        return;
    }

    if ((s_delay_test_measurement_active != 0U) &&
        ((jetson_link_online() == 0U) ||
         (jetson_link_ready() == 0U) ||
         (vision_sample_valid(now_ms) == 0U) ||
         (s_vision_received == 0U) ||
         ((uint32_t)(now_ms - s_vision.receive_time_ms) >
          GIMBAL_VISION_LOST_MS)))
    {
        delay_test_fail(
            jetson_link_online() != 0U ?
            GIMBAL_FAULT_VISION_LOST :
            GIMBAL_FAULT_JETSON_LINK,
            now_ms);
        return;
    }

    if ((s_delay_test_phase == DELAY_TEST_COUNTDOWN) &&
        (time_reached(now_ms, s_delay_test_deadline_ms) != 0U))
    {
        s_delay_test_phase = DELAY_TEST_READY_VALIDATE;
    }
    else if ((s_delay_test_phase == DELAY_TEST_PULSE) &&
             (time_reached(
                  now_ms, s_delay_test_deadline_ms) != 0U))
    {
        s_command_pipe_angle_mdeg = 0;
        s_requested_rad = s_level_zero_rad;
        s_command_rad = s_level_zero_rad;
        s_delay_test_phase = DELAY_TEST_RETURNING;
        s_delay_test_deadline_ms =
            now_ms + GIMBAL_DELAY_TEST_RETURN_TIMEOUT_MS;
        s_delay_test_return_settle_start_ms = 0U;
        s_next_motor_tx_ms = now_ms;
    }

    if (s_delay_test_phase == DELAY_TEST_PULSE)
    {
        float pipe_rad =
            ((float)GIMBAL_DELAY_TEST_PIPE_ANGLE_MDEG /
             1000.0f) * DEG_TO_RAD;
        desired_motor_rad =
            s_level_zero_rad -
            (pipe_rad / GIMBAL_PIPE_PER_MOTOR_RATIO);
    }
    else
    {
        desired_motor_rad = s_level_zero_rad;
    }
    s_requested_rad = desired_motor_rad;
    s_command_rad = desired_motor_rad;

    if (s_delay_test_phase == DELAY_TEST_RETURNING)
    {
        if ((absolute_float(
                 motor->position_rad - s_level_zero_rad) <=
             GIMBAL_DELAY_TEST_RETURN_POS_RAD) &&
            (absolute_float(motor->velocity_rad_s) <=
             GIMBAL_DELAY_TEST_RETURN_VEL_RAD_S))
        {
            if (s_delay_test_return_settle_start_ms == 0U)
            {
                s_delay_test_return_settle_start_ms = now_ms;
            }
            else if ((uint32_t)(
                         now_ms -
                         s_delay_test_return_settle_start_ms) >=
                     GIMBAL_DELAY_TEST_RETURN_SETTLE_MS)
            {
                s_delay_test_pulse_count++;
                if (s_delay_test_measurement_active != 0U)
                {
                    s_delay_test_measurement_count++;
                    s_delay_test_phase =
                        DELAY_TEST_READY_MEASURE;
                }
                else
                {
                    s_delay_test_validated = 1U;
                    s_delay_test_mechanical_ack = 0U;
                    s_delay_test_phase = DELAY_TEST_WAIT_ACK;
                }
                s_delay_test_measurement_active = 0U;
                s_delay_test_return_settle_start_ms = 0U;
            }
        }
        else
        {
            s_delay_test_return_settle_start_ms = 0U;
        }
        if ((s_delay_test_phase == DELAY_TEST_RETURNING) &&
            (time_reached(
                 now_ms, s_delay_test_deadline_ms) != 0U))
        {
            delay_test_fail(
                GIMBAL_FAULT_FEEDBACK_TIMEOUT, now_ms);
            return;
        }
    }

    if (time_reached(now_ms, s_next_motor_tx_ms) == 0U)
    {
        return;
    }
    if (dm4310_command_position_speed(
            desired_motor_rad,
            GIMBAL_DELAY_TEST_MOTOR_SPEED_RAD_S) != HAL_OK)
    {
        delay_test_fail(GIMBAL_FAULT_CAN_TX, now_ms);
        return;
    }
    s_next_motor_tx_ms =
        now_ms + GIMBAL_MOTOR_COMMAND_PERIOD_MS;
}
#endif

static void run_observe(uint32_t now_ms)
{
    const Dm4310Status *motor = dm4310_status();
    uint8_t post_jog_settling =
        ((s_post_jog_disable_deadline_ms != 0U) &&
         (time_reached(now_ms,
                       s_post_jog_disable_deadline_ms) == 0U)) ? 1U : 0U;

    /*
     * OBSERVE is a torque-disabled state. If fresh feedback says the drive is
     * enabled (state 1) or reports any fault, remove OUT1 power immediately.
     * This also guards against a drive that boots into an unexpected state.
     */
    if ((feedback_ready(now_ms) != 0U) &&
        (motor->error != 0U) &&
        !((motor->error == 1U) && (post_jog_settling != 0U)))
    {
        enter_stopped_state(GIMBAL_FAULT_MOTOR_REPORTED, now_ms);
        return;
    }

    if ((feedback_ready(now_ms) != 0U) && (motor->error == 0U))
    {
        s_post_jog_disable_deadline_ms = 0U;
    }

    if (time_reached(now_ms, s_next_motor_tx_ms) != 0U)
    {
        (void)dm4310_query();
        s_next_motor_tx_ms = now_ms + GIMBAL_OBSERVE_QUERY_PERIOD_MS;
    }
}

static void run_motion(uint32_t now_ms)
{
    const Dm4310Status *motor = dm4310_status();
    uint32_t elapsed = now_ms - s_state_start_ms;
    uint32_t timeout_ms = GIMBAL_HOLD_TIMEOUT_MS;
    float maximum_rad = active_motion_maximum_rad();
    float maximum_step =
        GIMBAL_DESKTOP_MAX_SLEW_RAD_S *
        ((float)GIMBAL_MOTOR_COMMAND_PERIOD_MS / 1000.0f);
    float delta;

    if (s_state == GIMBAL_STATE_STEP)
    {
        timeout_ms = (s_calrun_phase != CALRUN_IDLE) ?
                     GIMBAL_CALRUN_LEG_TIMEOUT_MS :
                     GIMBAL_STEP_TIMEOUT_MS;
    }
    else if (s_state == GIMBAL_STATE_SWEEP)
    {
        timeout_ms = GIMBAL_SWEEP_TIMEOUT_MS;
    }

    if (elapsed >= timeout_ms)
    {
        if (s_zero_referenced_motion != 0U)
        {
            int32_t position_mrad =
                (int32_t)(motor->position_rad * 1000.0f);
            int32_t offset_mrad =
                (int32_t)((motor->position_rad - s_level_zero_rad) *
                          1000.0f);

            if (dm4310_disable() != HAL_OK)
            {
                enter_stopped_state(GIMBAL_FAULT_CAN_TX, now_ms);
                return;
            }

            s_zero_referenced_motion = 0U;
            s_state = GIMBAL_STATE_OBSERVE;
            s_state_start_ms = now_ms;
            s_arm_deadline_ms = 0U;
            s_post_jog_disable_deadline_ms =
                now_ms + GIMBAL_POST_JOG_DISABLE_SETTLE_MS;
            s_next_motor_tx_ms = now_ms + GIMBAL_OBSERVE_QUERY_PERIOD_MS;
            (void)usb_console_printf(
                "JOG DONE P=%ldmrad OFF=%ldmrad; "
                "TORQUE OFF, OBSERVE, OUT1 ON\r\n",
                (long)position_mrad,
                (long)offset_mrad);
            return;
        }

        enter_stopped_state(GIMBAL_FAULT_NONE, now_ms);
        return;
    }

    if (feedback_ready(now_ms) == 0U)
    {
        enter_stopped_state(GIMBAL_FAULT_FEEDBACK_TIMEOUT, now_ms);
        return;
    }

    if ((motor->error != 0U) && (motor->error != 1U))
    {
        enter_stopped_state(GIMBAL_FAULT_MOTOR_REPORTED, now_ms);
        return;
    }

    if (time_reached(now_ms, s_next_motor_tx_ms) == 0U)
    {
        return;
    }

    if (s_state == GIMBAL_STATE_SWEEP)
    {
        if ((s_sweep_direction > 0) &&
            (s_command_rad >=
             (s_neutral_rad + s_sweep_amplitude_rad - maximum_step)))
        {
            s_sweep_direction = -1;
        }
        else if ((s_sweep_direction < 0) &&
                 (s_command_rad <=
                  (s_neutral_rad - s_sweep_amplitude_rad + maximum_step)))
        {
            s_sweep_direction = 1;
        }
        s_requested_rad = s_neutral_rad +
                          ((float)s_sweep_direction *
                           s_sweep_amplitude_rad);
    }

    s_requested_rad = clamp_float(s_requested_rad,
                                  s_neutral_rad - maximum_rad,
                                  s_neutral_rad + maximum_rad);
    delta = s_requested_rad - s_command_rad;
    if (delta > maximum_step)
    {
        delta = maximum_step;
    }
    else if (delta < -maximum_step)
    {
        delta = -maximum_step;
    }
    s_command_rad += delta;

    if (dm4310_command_position_speed(
            s_command_rad,
            GIMBAL_DESKTOP_MAX_SLEW_RAD_S) != HAL_OK)
    {
        enter_stopped_state(GIMBAL_FAULT_CAN_TX, now_ms);
        return;
    }
    s_next_motor_tx_ms = now_ms + GIMBAL_MOTOR_COMMAND_PERIOD_MS;
}

static void competition_fail(GimbalFault fault, uint32_t now_ms)
{
    enter_stopped_state(fault, now_ms);
    s_competition_phase = COMPETITION_FAULT;
    s_command_pipe_angle_mdeg = 0;
}

static int32_t competition_velocity_um_s(uint32_t now_ms)
{
    uint32_t latest_index;
    uint32_t selected_index;
    uint32_t sample_offset;
    uint32_t span_ms;

    (void)now_ms;
    if ((s_vision.receive_time_ms == 0U) ||
        (s_vision.receive_time_ms == s_velocity_last_sample_ms))
    {
        return s_velocity_filtered_um_s;
    }

    s_velocity_last_sample_ms = s_vision.receive_time_ms;
    s_velocity_history_um[s_velocity_history_head] =
        s_vision.position_um;
    s_velocity_history_ms[s_velocity_history_head] =
        s_vision.receive_time_ms;
    s_velocity_history_head =
        (s_velocity_history_head + 1U) %
        GIMBAL_VELOCITY_HISTORY_DEPTH;
    if (s_velocity_history_count <
        GIMBAL_VELOCITY_HISTORY_DEPTH)
    {
        s_velocity_history_count++;
    }

    latest_index =
        (s_velocity_history_head +
         GIMBAL_VELOCITY_HISTORY_DEPTH - 1U) %
        GIMBAL_VELOCITY_HISTORY_DEPTH;
    selected_index =
        (s_velocity_history_head +
         GIMBAL_VELOCITY_HISTORY_DEPTH -
         s_velocity_history_count) %
        GIMBAL_VELOCITY_HISTORY_DEPTH;

    for (sample_offset = 1U;
         sample_offset < s_velocity_history_count;
         sample_offset++)
    {
        uint32_t candidate_index =
            (latest_index +
             GIMBAL_VELOCITY_HISTORY_DEPTH -
             sample_offset) %
            GIMBAL_VELOCITY_HISTORY_DEPTH;
        uint32_t candidate_span_ms =
            s_velocity_history_ms[latest_index] -
            s_velocity_history_ms[candidate_index];

        selected_index = candidate_index;
        if (candidate_span_ms >= GIMBAL_VELOCITY_WINDOW_MS)
        {
            break;
        }
    }

    span_ms =
        s_velocity_history_ms[latest_index] -
        s_velocity_history_ms[selected_index];
    s_velocity_window_ms = span_ms;
    if ((span_ms >= GIMBAL_VELOCITY_WINDOW_MIN_MS) &&
        (span_ms <= GIMBAL_VELOCITY_WINDOW_MAX_MS))
    {
        int64_t measured_um_s =
            (((int64_t)s_velocity_history_um[latest_index] -
              (int64_t)s_velocity_history_um[selected_index]) *
             1000LL) /
            (int64_t)span_ms;

        if (measured_um_s > 2000000LL)
        {
            measured_um_s = 2000000LL;
        }
        else if (measured_um_s < -2000000LL)
        {
            measured_um_s = -2000000LL;
        }
        s_velocity_filtered_um_s = (int32_t)measured_um_s;
    }
    return s_velocity_filtered_um_s;
}

static int32_t competition_predict_position_um(
    int32_t velocity_um_s,
    uint32_t now_ms)
{
    uint32_t total_delay_ms = g_gimbal_debug.control_total_delay_ms;
    uint32_t sample_receive_age_ms;
    uint32_t horizon_ms;
    int64_t predicted_um;

    /*
     * total_delay_ms was measured from MC02 command to the corresponding
     * visual response, so it already contains Jetson processing and USB
     * transport. Only time elapsed after this sample reached MC02 is added.
     * Writing zero in Ozone deliberately disables the projection for an A/B
     * comparison. Invalid large writes fall back to the measured default.
     */
    if (total_delay_ms > GIMBAL_TOTAL_DELAY_MAX_MS)
    {
        total_delay_ms = GIMBAL_TOTAL_DELAY_MS;
    }
    if (total_delay_ms == 0U)
    {
        s_control_prediction_horizon_ms = 0U;
        return s_vision.position_um;
    }

    sample_receive_age_ms =
        (uint32_t)(now_ms - s_vision.receive_time_ms);
    if (sample_receive_age_ms > GIMBAL_VISION_STALE_MS)
    {
        sample_receive_age_ms = GIMBAL_VISION_STALE_MS;
    }
    horizon_ms =
        total_delay_ms +
        sample_receive_age_ms;
    if (horizon_ms > GIMBAL_TOTAL_DELAY_MAX_MS)
    {
        horizon_ms = GIMBAL_TOTAL_DELAY_MAX_MS;
    }

    predicted_um =
        (int64_t)s_vision.position_um +
        (((int64_t)velocity_um_s *
          (int64_t)horizon_ms) / 1000LL);
    if (predicted_um > GIMBAL_PREDICTED_POSITION_LIMIT_UM)
    {
        predicted_um = GIMBAL_PREDICTED_POSITION_LIMIT_UM;
    }
    else if (predicted_um < -GIMBAL_PREDICTED_POSITION_LIMIT_UM)
    {
        predicted_um = -GIMBAL_PREDICTED_POSITION_LIMIT_UM;
    }

    s_control_prediction_horizon_ms = horizon_ms;
    return (int32_t)predicted_um;
}

static void competition_q2_start_breakaway(
    int32_t pipe_angle_mdeg,
    uint32_t duration_ms,
    uint32_t now_ms)
{
    s_q2_breakaway_pipe_mdeg = pipe_angle_mdeg;
    s_q2_breakaway_deadline_ms =
        now_ms + duration_ms;
    s_q2_breakaway_count++;
    if (s_q2_breakaway_used_this_leg < 255U)
    {
        s_q2_breakaway_used_this_leg++;
    }
    competition_q2_stall_reset();
}

static void competition_q2_update_motion(uint32_t now_ms)
{
    int32_t span_um;

    if ((s_vision.receive_time_ms == 0U) ||
        (s_vision.receive_time_ms ==
         s_q2_motion_last_sample_ms))
    {
        return;
    }
    s_q2_motion_last_sample_ms = s_vision.receive_time_ms;

    if (s_q2_motion_window_start_ms == 0U)
    {
        s_q2_motion_window_start_ms = now_ms;
        s_q2_motion_min_um = s_vision.position_um;
        s_q2_motion_max_um = s_vision.position_um;
        s_q2_stationary_ms = 0U;
        return;
    }

    if (s_vision.position_um < s_q2_motion_min_um)
    {
        s_q2_motion_min_um = s_vision.position_um;
    }
    if (s_vision.position_um > s_q2_motion_max_um)
    {
        s_q2_motion_max_um = s_vision.position_um;
    }
    span_um = s_q2_motion_max_um - s_q2_motion_min_um;
    if (span_um > GIMBAL_Q2_STALL_SPAN_UM)
    {
        s_q2_motion_window_start_ms = now_ms;
        s_q2_motion_min_um = s_vision.position_um;
        s_q2_motion_max_um = s_vision.position_um;
        s_q2_stationary_ms = 0U;
        return;
    }

    s_q2_stationary_ms =
        now_ms - s_q2_motion_window_start_ms;
}

static void competition_q2_update_stall(uint32_t now_ms)
{
    int32_t target_error_um;
    int32_t span_um;
    uint8_t maximum_breakaway_count =
        s_competition_phase == COMPETITION_Q2_POSITIVE ?
            GIMBAL_Q2_POSITIVE_BREAKAWAY_MAX :
            GIMBAL_Q2_NEGATIVE_BREAKAWAY_MAX;

    if (((s_competition_phase != COMPETITION_Q2_POSITIVE) &&
         (s_competition_phase != COMPETITION_Q2_NEGATIVE)) ||
        (s_q2_breakaway_used_this_leg >=
         maximum_breakaway_count) ||
        (s_q2_breakaway_deadline_ms != 0U))
    {
        competition_q2_stall_reset();
        return;
    }

    if ((s_competition_phase == COMPETITION_Q2_POSITIVE) &&
        ((uint32_t)(now_ms - s_competition_start_ms) <
         GIMBAL_Q2_STALL_ARM_DELAY_MS))
    {
        competition_q2_stall_reset();
        return;
    }

    target_error_um =
        s_competition_target_um - s_vision.position_um;
    if ((target_error_um <= GIMBAL_TARGET_TOLERANCE_UM) &&
        (target_error_um >= -GIMBAL_TARGET_TOLERANCE_UM))
    {
        competition_q2_stall_reset();
        return;
    }
    /*
     * Only consume each received vision sample once. A 150 ms window whose
     * raw measured position stays within 2 mm means the ball is effectively
     * stationary even though the target is still more than 7 mm away.
     */
    if ((s_vision.receive_time_ms == 0U) ||
        (s_vision.receive_time_ms == s_q2_stall_last_sample_ms))
    {
        return;
    }
    s_q2_stall_last_sample_ms = s_vision.receive_time_ms;

    if (s_q2_stall_start_ms == 0U)
    {
        s_q2_stall_start_ms = now_ms;
        s_q2_stall_min_um = s_vision.position_um;
        s_q2_stall_max_um = s_vision.position_um;
        return;
    }

    if (s_vision.position_um < s_q2_stall_min_um)
    {
        s_q2_stall_min_um = s_vision.position_um;
    }
    if (s_vision.position_um > s_q2_stall_max_um)
    {
        s_q2_stall_max_um = s_vision.position_um;
    }
    span_um = s_q2_stall_max_um - s_q2_stall_min_um;
    if (span_um > GIMBAL_Q2_STALL_SPAN_UM)
    {
        s_q2_stall_start_ms = now_ms;
        s_q2_stall_min_um = s_vision.position_um;
        s_q2_stall_max_um = s_vision.position_um;
        return;
    }

    if ((uint32_t)(now_ms - s_q2_stall_start_ms) >=
        GIMBAL_Q2_STALL_WINDOW_MS)
    {
        int32_t breakaway_pipe_mdeg =
            s_competition_phase == COMPETITION_Q2_POSITIVE ?
                GIMBAL_Q2_POSITIVE_BREAKAWAY_PIPE_MDEG :
                GIMBAL_Q2_NEGATIVE_BREAKAWAY_PIPE_MDEG;
        uint32_t breakaway_ms =
            s_competition_phase == COMPETITION_Q2_POSITIVE ?
                GIMBAL_Q2_POSITIVE_BREAKAWAY_MS :
                GIMBAL_Q2_NEGATIVE_BREAKAWAY_MS;

        competition_q2_start_breakaway(
            target_error_um > 0 ?
                -breakaway_pipe_mdeg :
                 breakaway_pipe_mdeg,
            breakaway_ms,
            now_ms);
    }
}

static void competition_q2_begin_negative(uint32_t now_ms)
{
    s_competition_target_um =
        GIMBAL_Q2_NEGATIVE_TARGET_UM;
    s_competition_phase = COMPETITION_Q2_NEGATIVE;
    s_target_settle_start_ms = 0U;
    s_q2_breakaway_used_this_leg = 0U;
    competition_q2_stall_reset();
    competition_q2_start_breakaway(
        GIMBAL_Q2_NEGATIVE_BREAKAWAY_PIPE_MDEG,
        GIMBAL_Q2_NEGATIVE_BREAKAWAY_MS,
        now_ms);
}

static void competition_update_target_settle(
    int32_t velocity_um_s,
    int32_t predicted_position_um,
    uint32_t now_ms)
{
    int32_t error = s_competition_target_um - s_vision.position_um;
    int32_t velocity_tolerance_um_s =
        GIMBAL_TARGET_VELOCITY_TOL_UM_S;
    uint32_t settle_ms = GIMBAL_TARGET_SETTLE_MS;

    if (s_competition_phase == COMPETITION_Q2_POSITIVE)
    {
        /*
         * The printed task requires immediate return after reaching +50 mm.
         * Use the same bounded delay projection as the controller so the
         * return command takes effect close to the waypoint; raw position
         * remains an unconditional crossing fallback.
         */
        if ((s_vision.position_um >=
             GIMBAL_Q2_POSITIVE_TARGET_UM) ||
            (predicted_position_um >=
             GIMBAL_Q2_POSITIVE_TARGET_UM))
        {
            competition_q2_begin_negative(now_ms);
        }
        else
        {
            s_target_settle_start_ms = 0U;
        }
        return;
    }
    if (s_competition_phase == COMPETITION_Q2_NEGATIVE)
    {
        settle_ms = GIMBAL_Q2_FINAL_SETTLE_MS;
    }
    if (error < 0)
    {
        error = -error;
    }
    if (velocity_um_s < 0)
    {
        velocity_um_s = -velocity_um_s;
    }
    if ((error <= GIMBAL_TARGET_TOLERANCE_UM) &&
        (velocity_um_s <= velocity_tolerance_um_s))
    {
        if (s_target_settle_start_ms == 0U)
        {
            s_target_settle_start_ms = now_ms;
        }
        else if ((uint32_t)(now_ms - s_target_settle_start_ms) >=
                 settle_ms)
        {
            if (s_competition_phase == COMPETITION_Q2_NEGATIVE)
            {
                /*
                 * Hold active control through the printed five-second
                 * checkpoint; otherwise completing early lets TI remove
                 * torque just as the ball reaches the final point.
                 */
                if ((uint32_t)(now_ms - s_competition_start_ms) >=
                    GIMBAL_Q2_EARLIEST_COMPLETE_MS)
                {
                    s_competition_phase = COMPETITION_COMPLETE;
                    s_q2_breakaway_deadline_ms = 0U;
                    competition_q2_stall_reset();
                }
            }
        }
    }
    else
    {
        s_target_settle_start_ms = 0U;
    }
}

static void run_competition(uint32_t now_ms)
{
    const Dm4310Status *motor = dm4310_status();
    float desired_pipe_rad = 0.0f;
    float desired_motor_rad = s_level_zero_rad;
    float maximum_motor_rad =
        GIMBAL_COMP_MAX_MOTOR_ANGLE_DEG * DEG_TO_RAD;
    float pipe_per_motor = g_gimbal_debug.pipe_per_motor_ratio;
    float controller_kp = g_gimbal_debug.control_kp_s2;
    float controller_kd = g_gimbal_debug.control_kd_s;
    float chassis_forward_to_x_sign =
        g_gimbal_debug.control_chassis_forward_to_x_sign;
    float maximum_pipe_deg = g_gimbal_debug.max_pipe_angle_deg;
    float motor_slew = g_gimbal_debug.max_motor_slew_rad_s;
    float maximum_pipe_rad;
    g_gimbal_debug.control_chassis_accel_m_s2 = 0.0f;
    g_gimbal_debug.control_chassis_feedforward_mdeg = 0;
    if (pipe_per_motor < 0.01f || pipe_per_motor > 0.20f)
    {
        pipe_per_motor = GIMBAL_PIPE_PER_MOTOR_RATIO;
    }
    if (controller_kp < 0.1f || controller_kp > 20.0f)
    {
        controller_kp = GIMBAL_BALL_KP_S2;
    }
    if (controller_kd < 0.1f || controller_kd > 20.0f)
    {
        controller_kd = GIMBAL_BALL_KD_S;
    }
    if (chassis_forward_to_x_sign < -1.0f ||
        chassis_forward_to_x_sign > 1.0f)
    {
        chassis_forward_to_x_sign =
            GIMBAL_CHASSIS_FORWARD_TO_X_SIGN;
    }
    if (maximum_pipe_deg < 0.2f || maximum_pipe_deg > 5.0f)
    {
        maximum_pipe_deg = GIMBAL_COMP_MAX_PIPE_ANGLE_DEG;
    }
    if (motor_slew < 0.1f || motor_slew > 3.0f)
    {
        motor_slew = GIMBAL_COMP_MAX_SLEW_RAD_S;
    }
    maximum_pipe_rad = maximum_pipe_deg * DEG_TO_RAD;
    float maximum_step =
        motor_slew *
        ((float)GIMBAL_MOTOR_COMMAND_PERIOD_MS / 1000.0f);

    if (s_competition_question_id == 1U)
    {
        if ((s_competition_phase == COMPETITION_WAIT_VISION) &&
            (jetson_link_ready() != 0U) &&
            (vision_sample_valid(now_ms) != 0U))
        {
            s_competition_phase = COMPETITION_READY;
        }
        return;
    }
    if (s_competition_phase == COMPETITION_WAIT_POWER)
    {
        run_safe_transmit(now_ms);
        if (power_output_is_ready() == 0U)
        {
            return;
        }
        if (feedback_ready(now_ms) == 0U)
        {
            if (time_reached(now_ms, s_next_motor_tx_ms) != 0U)
            {
                (void)dm4310_query();
                s_next_motor_tx_ms =
                    now_ms + GIMBAL_OBSERVE_QUERY_PERIOD_MS;
            }
            return;
        }
        if (motor->error != 0U)
        {
            competition_fail(GIMBAL_FAULT_MOTOR_REPORTED, now_ms);
            return;
        }
        s_level_zero_rad = motor->position_rad;
        s_level_zero_valid = 1U;
        s_neutral_rad = s_level_zero_rad;
        s_requested_rad = s_level_zero_rad;
        s_command_rad = s_level_zero_rad;
        s_competition_phase = COMPETITION_WAIT_VISION;
        s_state = GIMBAL_STATE_COMPETITION_PREP;
    }

    if (s_competition_phase == COMPETITION_WAIT_VISION)
    {
        if ((jetson_link_ready() == 0U) ||
            (vision_sample_valid(now_ms) == 0U))
        {
            if (time_reached(now_ms, s_next_motor_tx_ms) != 0U)
            {
                (void)dm4310_query();
                s_next_motor_tx_ms =
                    now_ms + GIMBAL_OBSERVE_QUERY_PERIOD_MS;
            }
            return;
        }
        if (s_competition_question_id == 5U)
        {
            s_competition_target_um =
                s_vision.position_um;
            if (s_competition_target_um > 120000L)
            {
                s_competition_target_um = 120000L;
            }
            else if (s_competition_target_um < -120000L)
            {
                s_competition_target_um = -120000L;
            }
        }
        if (dm4310_enable() != HAL_OK)
        {
            competition_fail(GIMBAL_FAULT_CAN_TX, now_ms);
            return;
        }
        s_competition_phase = COMPETITION_READY;
        s_state = GIMBAL_STATE_COMPETITION;
        s_next_motor_tx_ms = now_ms;
    }

    if ((s_competition_phase == COMPETITION_FAULT) ||
        (s_competition_phase == COMPETITION_IDLE))
    {
        return;
    }
    if (feedback_ready(now_ms) == 0U)
    {
        competition_fail(GIMBAL_FAULT_FEEDBACK_TIMEOUT, now_ms);
        return;
    }
    if ((motor->error != 0U) && (motor->error != 1U))
    {
        competition_fail(GIMBAL_FAULT_MOTOR_REPORTED, now_ms);
        return;
    }
    if ((s_vision_received == 0U) ||
        ((uint32_t)(now_ms - s_vision.receive_time_ms) >
         GIMBAL_VISION_LOST_MS))
    {
        competition_fail(GIMBAL_FAULT_VISION_LOST, now_ms);
        return;
    }

    if ((s_competition_phase == COMPETITION_RUNNING) ||
        (s_competition_phase == COMPETITION_Q2_POSITIVE) ||
        (s_competition_phase == COMPETITION_Q2_NEGATIVE) ||
        (s_competition_phase == COMPETITION_COMPLETE))
    {
        if (vision_sample_valid(now_ms) != 0U)
        {
            int32_t velocity_um_s =
                competition_velocity_um_s(now_ms);
            int32_t predicted_position_um =
                competition_predict_position_um(
                    velocity_um_s,
                    now_ms);
            float error_m =
                ((float)(s_competition_target_um -
                         predicted_position_um)) / 1000000.0f;
            float velocity_m_s =
                (float)velocity_um_s /
                1000000.0f;
            s_control_predicted_position_um =
                predicted_position_um;
            s_control_velocity_um_s = velocity_um_s;
            competition_q2_update_motion(now_ms);
            float desired_acceleration =
                controller_kp * error_m -
                controller_kd * velocity_m_s;
            float chassis_acceleration_m_s2 =
                ti_link_chassis_acceleration_m_s2() *
                chassis_forward_to_x_sign;
            /*
             * Q2 is a stationary-chassis task.  Do not let a delayed or
             * residual TI ramp estimate bias either Q2 leg; Q3-Q5 retain the
             * unchanged acceleration feed-forward path below.
             */
            if (s_competition_question_id == 2U)
            {
                chassis_acceleration_m_s2 = 0.0f;
            }
            float chassis_feedforward_rad =
                -(GIMBAL_CHASSIS_ACCEL_FF_GAIN *
                  chassis_acceleration_m_s2) /
                GIMBAL_GRAVITY_M_S2;
            desired_pipe_rad =
                -(GIMBAL_ROLLING_INVERSE_FACTOR *
                  desired_acceleration) /
                GIMBAL_GRAVITY_M_S2 +
                chassis_feedforward_rad;
            g_gimbal_debug.control_chassis_accel_m_s2 =
                chassis_acceleration_m_s2;
            g_gimbal_debug.control_chassis_feedforward_mdeg =
                (int32_t)(chassis_feedforward_rad /
                          DEG_TO_RAD * 1000.0f);
            desired_pipe_rad =
                clamp_float(desired_pipe_rad,
                            -maximum_pipe_rad,
                            maximum_pipe_rad);
            if ((s_competition_phase ==
                 COMPETITION_Q2_POSITIVE) ||
                (s_competition_phase ==
                 COMPETITION_Q2_NEGATIVE))
            {
                int32_t target_error_um =
                    s_competition_target_um -
                    s_vision.position_um;
                int32_t absolute_target_error_um =
                    target_error_um;
                uint8_t ball_is_stationary =
                    s_q2_stationary_ms >=
                    GIMBAL_Q2_STATIONARY_MS ? 1U : 0U;
                uint8_t final_capture_active = 0U;
                float minimum_drive_rad =
                    ((float)GIMBAL_Q2_MIN_DRIVE_PIPE_MDEG /
                     1000.0f) * DEG_TO_RAD;
                if (absolute_target_error_um < 0)
                {
                    absolute_target_error_um =
                        -absolute_target_error_um;
                }
                if ((s_competition_phase ==
                     COMPETITION_Q2_NEGATIVE) &&
                    (absolute_target_error_um <=
                     GIMBAL_Q2_FINAL_CAPTURE_ERROR_UM))
                {
                    s_q2_final_capture_latched = 1U;
                }
                final_capture_active =
                    s_q2_final_capture_latched;
                /*
                 * The bounded drive floor is admitted only after position
                 * remains inside a 2 mm band for 120 ms. A single noisy
                 * low-speed reading can no longer override normal PD
                 * braking while the ball is moving.
                 */
                if ((target_error_um >
                     GIMBAL_TARGET_TOLERANCE_UM) &&
                    (final_capture_active == 0U) &&
                    (ball_is_stationary != 0U) &&
                    (desired_pipe_rad > -minimum_drive_rad))
                {
                    desired_pipe_rad = -minimum_drive_rad;
                }
                else if ((target_error_um <
                         -GIMBAL_TARGET_TOLERANCE_UM) &&
                         (final_capture_active == 0U) &&
                         (ball_is_stationary != 0U) &&
                         (desired_pipe_rad < minimum_drive_rad))
                {
                    desired_pipe_rad = minimum_drive_rad;
                }

                if (s_competition_phase ==
                    COMPETITION_Q2_POSITIVE)
                {
                    float positive_keep_drive_rad =
                        ((float)GIMBAL_Q2_POSITIVE_KEEP_DRIVE_MDEG /
                         1000.0f) * DEG_TO_RAD;
                    float positive_low_speed_rad =
                        ((float)GIMBAL_Q2_POSITIVE_LOW_SPEED_MDEG /
                         1000.0f) * DEG_TO_RAD;

                    /*
                     * The +50 mm point is only an immediate-turn waypoint.
                     * Hardware logs show that early velocity damping can stop
                     * the ball at +21..+28 mm, where even two 1.5-degree
                     * pulses may not restart it. Keep a small forward tilt
                     * before +30 mm, and use the full fixed 2-degree envelope
                     * only after the ball remains in a 2 mm band for 120 ms
                     * while still outside the waypoint band. Normal PD
                     * braking is restored after +30 mm so the controller can
                     * remove energy before the immediate +50 mm threshold.
                     */
                    if ((target_error_um >
                         GIMBAL_TARGET_TOLERANCE_UM) &&
                        (ball_is_stationary != 0U) &&
                        (desired_pipe_rad >
                         -positive_low_speed_rad))
                    {
                        desired_pipe_rad = -positive_low_speed_rad;
                    }
                    else if ((s_vision.position_um <
                              GIMBAL_Q2_POSITIVE_NO_BRAKE_END_UM) &&
                             (desired_pipe_rad >
                              -positive_keep_drive_rad))
                    {
                        desired_pipe_rad =
                            -positive_keep_drive_rad;
                    }

                }
            }
            competition_update_target_settle(
                velocity_um_s,
                predicted_position_um,
                now_ms);
            competition_q2_update_stall(now_ms);
            if (((s_competition_phase ==
                  COMPETITION_Q2_POSITIVE) ||
                 (s_competition_phase ==
                  COMPETITION_Q2_NEGATIVE)) &&
                (s_q2_breakaway_deadline_ms != 0U))
            {
                if (time_reached(
                        now_ms,
                        s_q2_breakaway_deadline_ms) == 0U)
                {
                    /*
                     * A bounded, directional kick overcomes static friction.
                     * Positive Q2 may use a second separately detected kick;
                     * negative Q2 remains limited to one. It is applied only
                     * with a valid vision sample.
                     */
                    desired_pipe_rad =
                        ((float)s_q2_breakaway_pipe_mdeg /
                         1000.0f) * DEG_TO_RAD;
                }
                else
                {
                    s_q2_breakaway_deadline_ms = 0U;
                }
            }
            if (s_competition_phase == COMPETITION_Q2_NEGATIVE)
            {
                float capture_pipe_rad =
                    ((float)GIMBAL_Q2_FINAL_CAPTURE_PIPE_MDEG /
                     1000.0f) * DEG_TO_RAD;
                int32_t absolute_velocity_um_s =
                    velocity_um_s;
                if (absolute_velocity_um_s < 0)
                {
                    absolute_velocity_um_s =
                        -absolute_velocity_um_s;
                }
                if (s_q2_final_capture_latched != 0U)
                {
                    /*
                     * A pipe command with the same sign as ball velocity
                     * removes kinetic energy, so it retains full bounded
                     * authority. Only non-braking drive is reduced after
                     * final capture is latched.
                     */
                    if ((absolute_velocity_um_s <= 20000L) ||
                        ((desired_pipe_rad * velocity_m_s) <= 0.0f))
                    {
                        desired_pipe_rad =
                            clamp_float(desired_pipe_rad,
                                        -capture_pipe_rad,
                                        capture_pipe_rad);
                    }
                }

                if (s_q2_breakaway_deadline_ms == 0U)
                {
                    float negative_drive_cap_rad =
                        ((float)GIMBAL_Q2_NEGATIVE_DRIVE_CAP_MDEG /
                         1000.0f) * DEG_TO_RAD;
                    /*
                     * After the single transition pulse, positive-pipe drive
                     * stays capped while the ball moves negative or remains
                     * in the velocity-noise band. At >=30 mm/s positive
                     * rebound the same request is braking and regains the
                     * full fixed two-degree authority.
                     */
                    if ((desired_pipe_rad >
                         negative_drive_cap_rad) &&
                        (velocity_um_s <
                         GIMBAL_Q2_REBOUND_BRAKE_UM_S))
                    {
                        desired_pipe_rad =
                            negative_drive_cap_rad;
                    }
                }
            }
        }
        else
        {
            /* A stale/invalid frame always returns the requested tilt to 0. */
            desired_pipe_rad = 0.0f;
            s_target_settle_start_ms = 0U;
            s_q2_breakaway_deadline_ms = 0U;
            competition_q2_stall_reset();
            s_control_prediction_horizon_ms = 0U;
            s_control_predicted_position_um = s_vision.position_um;
            s_control_velocity_um_s = 0;
            competition_velocity_reset();
            competition_q2_motion_reset();
        }
    }

    if ((s_competition_question_id == 2U) &&
        (s_competition_start_ms != 0U) &&
        ((uint32_t)(now_ms - s_competition_start_ms) >
         GIMBAL_Q2_TIMEOUT_MS) &&
        (s_competition_phase != COMPETITION_COMPLETE))
    {
        competition_fail(GIMBAL_FAULT_BAD_COMMAND, now_ms);
        return;
    }

    s_command_pipe_angle_mdeg =
        (int32_t)(desired_pipe_rad / DEG_TO_RAD * 1000.0f);
    desired_motor_rad =
        s_level_zero_rad -
        (desired_pipe_rad / pipe_per_motor);
    desired_motor_rad =
        clamp_float(desired_motor_rad,
                    s_level_zero_rad - maximum_motor_rad,
                    s_level_zero_rad + maximum_motor_rad);
    s_requested_rad = desired_motor_rad;

    if (time_reached(now_ms, s_next_motor_tx_ms) == 0U)
    {
        return;
    }
    {
        float delta = desired_motor_rad - s_command_rad;
        if (delta > maximum_step)
        {
            delta = maximum_step;
        }
        else if (delta < -maximum_step)
        {
            delta = -maximum_step;
        }
        s_command_rad += delta;
    }
    if (dm4310_command_position_speed(
            s_command_rad, motor_slew) != HAL_OK)
    {
        competition_fail(GIMBAL_FAULT_CAN_TX, now_ms);
        return;
    }
    s_next_motor_tx_ms = now_ms + GIMBAL_MOTOR_COMMAND_PERIOD_MS;
}

static void update_debug(uint32_t now_ms)
{
    const Dm4310Status *motor = dm4310_status();

    g_gimbal_debug.build_id = GIMBAL_BUILD_ID;
    g_gimbal_debug.uptime_ms = now_ms;
    g_gimbal_debug.state = (uint32_t)s_state;
    g_gimbal_debug.fault = (uint32_t)s_fault;
    g_gimbal_debug.state_elapsed_ms = now_ms - s_state_start_ms;
    g_gimbal_debug.arm_remaining_ms =
        (arm_is_valid(now_ms) != 0U) ?
        (uint32_t)(s_arm_deadline_ms - now_ms) : 0U;
    g_gimbal_debug.power_output_enabled = s_power_output_enabled;
    g_gimbal_debug.power_arm_remaining_ms =
        (power_arm_is_valid(now_ms) != 0U) ?
        (uint32_t)(s_power_arm_deadline_ms - now_ms) : 0U;
    g_gimbal_debug.power_on_remaining_ms =
        ((s_power_output_enabled != 0U) &&
         (time_reached(now_ms, s_power_off_deadline_ms) == 0U)) ?
        (uint32_t)(s_power_off_deadline_ms - now_ms) : 0U;
    g_gimbal_debug.calrun_phase = (uint32_t)s_calrun_phase;
#if GIMBAL_ENABLE_DELAY_TEST
    g_gimbal_debug.delay_test_enabled = 1U;
    g_gimbal_debug.delay_test_phase =
        (uint32_t)s_delay_test_phase;
    g_gimbal_debug.delay_test_arm_remaining_ms =
        (delay_test_arm_is_valid(now_ms) != 0U) ?
        (uint32_t)(s_delay_test_arm_deadline_ms - now_ms) : 0U;
    g_gimbal_debug.delay_test_validated =
        s_delay_test_validated;
    g_gimbal_debug.delay_test_mechanical_ack =
        s_delay_test_mechanical_ack;
    g_gimbal_debug.delay_test_measurement_active =
        s_delay_test_measurement_active;
    g_gimbal_debug.delay_test_pulse_count =
        s_delay_test_pulse_count;
    g_gimbal_debug.delay_test_measurement_count =
        s_delay_test_measurement_count;
    g_gimbal_debug.delay_test_denied_count =
        s_delay_test_denied_count;
    g_gimbal_debug.delay_test_last_denied_request =
        s_delay_test_last_denied_request;
    g_gimbal_debug.delay_test_last_pulse_start_ms =
        s_delay_test_last_pulse_start_ms;
    g_gimbal_debug.delay_test_stable_ready =
        s_delay_test_stable_ready;
    g_gimbal_debug.delay_test_stable_elapsed_ms =
        s_delay_test_stable_start_ms != 0U ?
        (uint32_t)(now_ms - s_delay_test_stable_start_ms) : 0U;
    g_gimbal_debug.delay_test_stable_span_um =
        (s_delay_test_stable_sample_count != 0U) ?
        (uint32_t)(s_delay_test_stable_max_um -
                   s_delay_test_stable_min_um) : 0U;
    g_gimbal_debug.delay_test_stable_sample_count =
        s_delay_test_stable_sample_count;
#else
    g_gimbal_debug.delay_test_enabled = 0U;
#endif
    g_gimbal_debug.delay_test_fixed_pipe_angle_mdeg =
        GIMBAL_DELAY_TEST_PIPE_ANGLE_MDEG;
    g_gimbal_debug.delay_test_fixed_motor_speed_rad_s =
        GIMBAL_DELAY_TEST_MOTOR_SPEED_RAD_S;

    g_gimbal_debug.level_zero_valid = s_level_zero_valid;
    g_gimbal_debug.level_zero_position_rad = s_level_zero_rad;
    g_gimbal_debug.motor_offset_from_level_rad =
        (s_level_zero_valid != 0U) ?
        (motor->position_rad - s_level_zero_rad) : 0.0f;
    g_gimbal_debug.neutral_position_rad = s_neutral_rad;
    g_gimbal_debug.requested_position_rad = s_requested_rad;
    g_gimbal_debug.command_position_rad = s_command_rad;
    g_gimbal_debug.feedback_position_rad = motor->position_rad;
    g_gimbal_debug.feedback_velocity_rad_s = motor->velocity_rad_s;
    g_gimbal_debug.feedback_torque_nm = motor->torque_nm;
    g_gimbal_debug.feedback_age_ms =
        (motor->rx_count == 0U) ?
        0xFFFFFFFFUL : (uint32_t)(now_ms - motor->last_rx_ms);
    g_gimbal_debug.motor_error = motor->error;
    g_gimbal_debug.motor_mos_temperature_c =
        motor->mos_temperature_c;
    g_gimbal_debug.motor_rotor_temperature_c =
        motor->rotor_temperature_c;

    g_gimbal_debug.can_rx_count = motor->rx_count;
    g_gimbal_debug.can_tx_count = motor->tx_count;
    g_gimbal_debug.can_tx_error_count = motor->tx_error_count;
    g_gimbal_debug.usb_rx_bytes = usb_console_rx_bytes();
    g_gimbal_debug.usb_rx_overflow = usb_console_rx_overflow();
    g_gimbal_debug.usb_tx_drop = usb_console_tx_drop();

    if (s_vision_received != 0U)
    {
        g_gimbal_debug.vision_position_um = s_vision.position_um;
        g_gimbal_debug.vision_velocity_um_s = s_vision.velocity_um_s;
        g_gimbal_debug.vision_processing_latency_us =
            s_vision.processing_latency_us;
        g_gimbal_debug.vision_age_ms =
            (uint32_t)(now_ms - s_vision.receive_time_ms);
        g_gimbal_debug.vision_flags = s_vision.flags;
    }
    else
    {
        g_gimbal_debug.vision_age_ms = 0xFFFFFFFFUL;
    }
    g_gimbal_debug.vision_closed_loop_enabled = 0U;
    if (((s_competition_phase == COMPETITION_RUNNING) ||
         (s_competition_phase == COMPETITION_Q2_POSITIVE) ||
         (s_competition_phase == COMPETITION_Q2_NEGATIVE) ||
         (s_competition_phase == COMPETITION_COMPLETE)) &&
        (vision_sample_valid(now_ms) != 0U))
    {
        g_gimbal_debug.vision_closed_loop_enabled = 1U;
    }
    g_gimbal_debug.competition_run_id = s_competition_run_id;
    g_gimbal_debug.competition_question_id =
        s_competition_question_id;
    g_gimbal_debug.competition_phase =
        (uint32_t)s_competition_phase;
    g_gimbal_debug.competition_target_um =
        s_competition_target_um;
    g_gimbal_debug.command_pipe_angle_mdeg =
        s_command_pipe_angle_mdeg;
    g_gimbal_debug.target_settle_ms =
        s_target_settle_start_ms != 0U ?
        (uint32_t)(now_ms - s_target_settle_start_ms) : 0U;
    g_gimbal_debug.q2_breakaway_active = 0U;
    g_gimbal_debug.q2_breakaway_remaining_ms = 0U;
    if ((s_q2_breakaway_deadline_ms != 0U) &&
        (time_reached(now_ms, s_q2_breakaway_deadline_ms) == 0U))
    {
        g_gimbal_debug.q2_breakaway_active = 1U;
        g_gimbal_debug.q2_breakaway_remaining_ms =
            s_q2_breakaway_deadline_ms - now_ms;
    }
    g_gimbal_debug.q2_breakaway_count =
        s_q2_breakaway_count;
    g_gimbal_debug.q2_breakaway_pipe_angle_mdeg =
        s_q2_breakaway_pipe_mdeg;
    g_gimbal_debug.q2_breakaway_used_this_leg =
        s_q2_breakaway_used_this_leg;
    g_gimbal_debug.q2_stall_elapsed_ms =
        s_q2_stall_start_ms != 0U ?
        (uint32_t)(now_ms - s_q2_stall_start_ms) : 0U;
    g_gimbal_debug.q2_stall_span_um =
        s_q2_stall_start_ms != 0U ?
        s_q2_stall_max_um - s_q2_stall_min_um : 0;
    g_gimbal_debug.control_prediction_horizon_ms =
        s_control_prediction_horizon_ms;
    g_gimbal_debug.control_predicted_position_um =
        s_control_predicted_position_um;
    g_gimbal_debug.control_velocity_um_s =
        s_control_velocity_um_s;
    g_gimbal_debug.control_velocity_window_ms =
        s_velocity_window_ms;
    g_gimbal_debug.q2_stationary_ms =
        s_q2_stationary_ms;
    g_gimbal_debug.q2_final_capture_active =
        s_q2_final_capture_latched;
}

void gimbal_app_init(uint32_t now_ms)
{
    const Dm4310Status *motor;

    memset((void *)&g_gimbal_debug, 0, sizeof(g_gimbal_debug));
    memset(&s_vision, 0, sizeof(s_vision));
    s_vision_received = 0U;
    s_competition_phase = COMPETITION_IDLE;
    s_competition_run_id = 0U;
    s_competition_start_ms = 0U;
    s_target_settle_start_ms = 0U;
    s_q2_breakaway_deadline_ms = 0U;
    s_q2_breakaway_count = 0U;
    s_q2_breakaway_pipe_mdeg = 0;
    s_q2_breakaway_used_this_leg = 0U;
    s_q2_final_capture_latched = 0U;
    competition_q2_stall_reset();
    s_competition_question_id = 0U;
    s_competition_target_um = 0;
    s_command_pipe_angle_mdeg = 0;
    competition_velocity_reset();
    competition_q2_motion_reset();
    s_control_prediction_horizon_ms = 0U;
    s_control_predicted_position_um = 0;
    s_control_velocity_um_s = 0;
#if GIMBAL_ENABLE_DELAY_TEST
    s_delay_test_phase = DELAY_TEST_IDLE;
    s_delay_test_deadline_ms = 0U;
    s_delay_test_arm_deadline_ms = 0U;
    s_delay_test_return_settle_start_ms = 0U;
    s_delay_test_validated = 0U;
    s_delay_test_mechanical_ack = 0U;
    s_delay_test_measurement_active = 0U;
    s_delay_test_pulse_count = 0U;
    s_delay_test_measurement_count = 0U;
    s_delay_test_denied_count = 0U;
    s_delay_test_last_denied_request = 0U;
    s_delay_test_last_pulse_start_ms = 0U;
    delay_test_stability_reset();
#endif
    g_gimbal_debug.control_kp_s2 = GIMBAL_BALL_KP_S2;
    g_gimbal_debug.control_kd_s = GIMBAL_BALL_KD_S;
    g_gimbal_debug.control_chassis_forward_to_x_sign =
        GIMBAL_CHASSIS_FORWARD_TO_X_SIGN;
    g_gimbal_debug.control_chassis_accel_m_s2 = 0.0f;
    g_gimbal_debug.control_chassis_feedforward_mdeg = 0;
    g_gimbal_debug.control_total_delay_ms =
        GIMBAL_TOTAL_DELAY_MS;
    g_gimbal_debug.pipe_per_motor_ratio =
        GIMBAL_PIPE_PER_MOTOR_RATIO;
    g_gimbal_debug.max_pipe_angle_deg =
        GIMBAL_COMP_MAX_PIPE_ANGLE_DEG;
    g_gimbal_debug.max_motor_slew_rad_s =
        GIMBAL_COMP_MAX_SLEW_RAD_S;
    s_state = GIMBAL_STATE_BOOT;
    s_fault = GIMBAL_FAULT_NONE;
    s_state_start_ms = now_ms;
    s_arm_deadline_ms = 0U;
    s_power_arm_deadline_ms = 0U;
    s_power_off_deadline_ms = 0U;
    s_post_jog_disable_deadline_ms = 0U;
    s_power_output_enabled = 0U;
    s_level_zero_valid = 0U;
    s_zero_referenced_motion = 0U;
    s_calrun_phase = CALRUN_IDLE;
    s_calrun_deadline_ms = 0U;
    s_calrun_amplitude_tenths_deg = 0;
    s_level_zero_rad = 0.0f;
    power_output_force_off();
    s_neutral_rad = 0.0f;
    s_requested_rad = 0.0f;
    s_command_rad = 0.0f;

    dm4310_init(&hfdcan1, now_ms);
    motor = dm4310_status();
    if (motor->initialized == 0U)
    {
        enter_stopped_state(GIMBAL_FAULT_CAN_INIT, now_ms);
    }
    else
    {
        enter_stopped_state(GIMBAL_FAULT_NONE, now_ms);
    }
    update_debug(now_ms);
}

void gimbal_app_poll(uint32_t now_ms)
{
    uint32_t request;
    int32_t parameter;
    uint8_t debugger_arm;

    dm4310_poll(now_ms);
    if (jetson_link_online() == 0U)
    {
        process_console(now_ms);
    }
    else
    {
        usb_console_discard_rx();
    }

    if ((s_power_output_enabled != 0U) &&
        (time_reached(now_ms, s_power_off_deadline_ms) != 0U))
    {
#if GIMBAL_ENABLE_DELAY_TEST
        if (delay_test_mode_owns_app() != 0U)
        {
            delay_test_fail(
                GIMBAL_FAULT_FEEDBACK_TIMEOUT, now_ms);
        }
        else
#endif
        if (s_competition_run_id != 0U)
        {
            competition_fail(GIMBAL_FAULT_FEEDBACK_TIMEOUT, now_ms);
        }
        else
        {
            enter_stopped_state(GIMBAL_FAULT_NONE, now_ms);
        }
    }

    request = g_gimbal_debug.request;
    parameter = g_gimbal_debug.request_param_tenths_deg;
    debugger_arm =
        (g_gimbal_debug.arm_key == GIMBAL_DEBUG_ARM_KEY) ? 1U : 0U;
#if GIMBAL_ENABLE_DELAY_TEST
    delay_test_process_debug_request(now_ms);
    if (delay_test_mode_owns_app() != 0U)
    {
        g_gimbal_debug.arm_key = 0U;
        if (request != GIMBAL_REQUEST_NONE)
        {
            g_gimbal_debug.request = GIMBAL_REQUEST_NONE;
            if (request == GIMBAL_REQUEST_STOP)
            {
                enter_stopped_state(
                    GIMBAL_FAULT_NONE, now_ms);
            }
            request = GIMBAL_REQUEST_NONE;
        }
    }
#else
    g_gimbal_debug.delay_test_key = 0U;
    g_gimbal_debug.delay_test_request =
        GIMBAL_DELAY_REQUEST_NONE;
#endif
    if ((s_competition_run_id != 0U) &&
        (request != GIMBAL_REQUEST_NONE))
    {
        g_gimbal_debug.request = GIMBAL_REQUEST_NONE;
        g_gimbal_debug.arm_key = 0U;
        if (request == GIMBAL_REQUEST_STOP)
        {
            gimbal_app_stop_question(
                s_competition_run_id, 0U, now_ms);
        }
    }
    else if (s_calrun_phase != CALRUN_IDLE)
    {
        g_gimbal_debug.request = GIMBAL_REQUEST_NONE;
        g_gimbal_debug.arm_key = 0U;
        if (request == GIMBAL_REQUEST_STOP)
        {
            process_request(GIMBAL_REQUEST_STOP, 0, 0U, now_ms);
        }
    }
    else if (request != GIMBAL_REQUEST_NONE)
    {
        g_gimbal_debug.request = GIMBAL_REQUEST_NONE;
        g_gimbal_debug.arm_key = 0U;
        process_request((GimbalRequest)request,
                        parameter,
                        debugger_arm,
                        now_ms);
    }
    else if (g_gimbal_debug.arm_key == GIMBAL_DEBUG_ARM_KEY)
    {
        /*
         * Ozone Watch writes are not atomic as a group.  Accept the key by
         * itself and retain the resulting three-second arm window so the
         * operator can write request afterwards.
         */
        arm_for_motion(now_ms);
        g_gimbal_debug.arm_key = 0U;
    }
    else if (g_gimbal_debug.arm_key != 0U)
    {
        /* A key written without a request is never retained. */
        g_gimbal_debug.arm_key = 0U;
    }

    if (s_competition_run_id == 0U
#if GIMBAL_ENABLE_DELAY_TEST
        && (delay_test_mode_owns_app() == 0U)
#endif
       )
    {
        run_calrun(now_ms);
    }

#if GIMBAL_ENABLE_DELAY_TEST
    if (delay_test_mode_owns_app() != 0U)
    {
        run_delay_test(now_ms);
    }
    else
#endif
    if ((s_competition_run_id != 0U) &&
        (s_competition_phase != COMPETITION_IDLE) &&
        (s_competition_phase != COMPETITION_FAULT))
    {
        run_competition(now_ms);
    }
    else if (motion_active() != 0U)
    {
        run_motion(now_ms);
    }
    else if (s_state == GIMBAL_STATE_OBSERVE)
    {
        run_observe(now_ms);
    }
    else
    {
        run_safe_transmit(now_ms);
    }

    update_debug(now_ms);
}

void gimbal_app_on_vision_sample(const VisionSample *sample)
{
    if (sample == NULL)
    {
        return;
    }
    s_vision = *sample;
    s_vision_received = 1U;
    /*
     * Keep the new observation diagnostics live before a Q2 run so the
     * operator can verify velocity noise and the stationary window before
     * granting motor motion. The receive-time guards make the later control
     * call consume the same sample without inserting it twice.
     */
    if (vision_sample_valid(sample->receive_time_ms) != 0U)
    {
        s_control_velocity_um_s =
            competition_velocity_um_s(sample->receive_time_ms);
        competition_q2_update_motion(sample->receive_time_ms);
    }
    else
    {
        s_control_velocity_um_s = 0;
        competition_velocity_reset();
        competition_q2_motion_reset();
    }
#if GIMBAL_ENABLE_DELAY_TEST
    delay_test_stability_on_sample(sample);
#endif
}

uint8_t gimbal_app_begin_question(
    uint32_t run_id, uint8_t question_id, uint32_t now_ms)
{
    if ((run_id == 0U) || (question_id < 1U) ||
        (question_id > 5U) ||
        (s_competition_run_id != 0U) ||
        (s_state != GIMBAL_STATE_SAFE_IDLE) ||
        (s_fault != GIMBAL_FAULT_NONE))
    {
        return 0U;
    }

    s_competition_run_id = run_id;
    s_competition_question_id = question_id;
    s_competition_start_ms = 0U;
    s_target_settle_start_ms = 0U;
    s_q2_breakaway_deadline_ms = 0U;
    s_q2_breakaway_count = 0U;
    s_q2_breakaway_pipe_mdeg = 0;
    s_q2_breakaway_used_this_leg = 0U;
    s_q2_final_capture_latched = 0U;
    competition_q2_stall_reset();
    s_competition_target_um = 0;
    s_command_pipe_angle_mdeg = 0;
    competition_velocity_reset();
    competition_q2_motion_reset();

    if (question_id == 1U)
    {
        s_competition_phase = COMPETITION_WAIT_VISION;
        s_state = GIMBAL_STATE_COMPETITION_PREP;
        return 1U;
    }

    if ((arm_power_output(now_ms) == 0U) ||
        (power_output_enable(now_ms) == 0U))
    {
        s_competition_run_id = 0U;
        s_competition_question_id = 0U;
        s_competition_phase = COMPETITION_IDLE;
        return 0U;
    }
    s_competition_phase = COMPETITION_WAIT_POWER;
    s_state = GIMBAL_STATE_COMPETITION_PREP;
    s_state_start_ms = now_ms;
    return 1U;
}

uint8_t gimbal_app_question_ready(uint32_t run_id)
{
    return ((run_id != 0U) &&
            (run_id == s_competition_run_id) &&
            (s_competition_phase == COMPETITION_READY)) ? 1U : 0U;
}

void gimbal_app_start_question(uint32_t run_id, uint32_t now_ms)
{
    if ((run_id == 0U) || (run_id != s_competition_run_id) ||
        (s_competition_phase != COMPETITION_READY))
    {
        return;
    }
    s_competition_start_ms = now_ms;
    s_target_settle_start_ms = 0U;
    s_q2_breakaway_deadline_ms = 0U;
    s_q2_breakaway_pipe_mdeg = 0;
    s_q2_breakaway_used_this_leg = 0U;
    s_q2_final_capture_latched = 0U;
    competition_q2_stall_reset();
    competition_q2_motion_reset();
    if (s_competition_question_id == 2U)
    {
        s_competition_target_um =
            GIMBAL_Q2_POSITIVE_TARGET_UM;
        s_competition_phase = COMPETITION_Q2_POSITIVE;
    }
    else
    {
        s_competition_phase = COMPETITION_RUNNING;
    }
}

uint8_t gimbal_app_question_complete(uint32_t run_id)
{
    return ((run_id != 0U) &&
            (run_id == s_competition_run_id) &&
            (s_competition_phase == COMPETITION_COMPLETE)) ? 1U : 0U;
}

uint8_t gimbal_app_question_faulted(uint32_t run_id)
{
    return ((run_id != 0U) &&
            (run_id == s_competition_run_id) &&
            ((s_competition_phase == COMPETITION_FAULT) ||
             (s_fault != GIMBAL_FAULT_NONE))) ? 1U : 0U;
}

uint8_t gimbal_app_ball_within_route_settle_limits(uint32_t now_ms)
{
    if (s_competition_phase != COMPETITION_RUNNING ||
        vision_sample_valid(now_ms) == 0U)
    {
        return 0U;
    }

    int32_t position_error_um =
        s_competition_target_um - s_vision.position_um;
    int32_t velocity_um_s = s_control_velocity_um_s;
    if (position_error_um < 0)
    {
        position_error_um = -position_error_um;
    }
    if (velocity_um_s < 0)
    {
        velocity_um_s = -velocity_um_s;
    }
    return (position_error_um <= GIMBAL_ROUTE_SETTLE_POSITION_UM &&
            velocity_um_s <= GIMBAL_ROUTE_SETTLE_VELOCITY_UM_S) ? 1U : 0U;
}

void gimbal_app_complete_question(uint32_t run_id, uint32_t now_ms)
{
    if ((run_id == 0U) || (run_id != s_competition_run_id))
    {
        return;
    }
    enter_stopped_state(GIMBAL_FAULT_NONE, now_ms);
    s_competition_phase = COMPETITION_COMPLETE;
}

void gimbal_app_stop_question(
    uint32_t run_id, uint8_t faulted, uint32_t now_ms)
{
    if ((s_competition_run_id == 0U) ||
        ((run_id != 0U) && (run_id != s_competition_run_id)))
    {
        return;
    }
    enter_stopped_state(
        faulted != 0U ?
            (jetson_link_online() != 0U ?
             GIMBAL_FAULT_VISION_LOST :
             GIMBAL_FAULT_JETSON_LINK) :
            GIMBAL_FAULT_NONE,
        now_ms);
    if (faulted == 0U)
    {
        s_competition_phase = COMPETITION_IDLE;
    }
    s_competition_run_id = 0U;
    s_competition_question_id = 0U;
    s_competition_start_ms = 0U;
    s_target_settle_start_ms = 0U;
    s_q2_breakaway_deadline_ms = 0U;
    s_q2_final_capture_latched = 0U;
    competition_q2_stall_reset();
    s_competition_target_um = 0;
}

uint8_t gimbal_app_system_state(void)
{
    if ((s_fault != GIMBAL_FAULT_NONE) ||
        (s_competition_phase == COMPETITION_FAULT))
    {
        return 7U;
    }
#if GIMBAL_ENABLE_DELAY_TEST
    if ((s_delay_test_phase == DELAY_TEST_WAIT_POWER) ||
        (s_delay_test_phase == DELAY_TEST_COUNTDOWN))
    {
        return 2U;
    }
    if ((s_delay_test_phase >= DELAY_TEST_READY_VALIDATE) &&
        (s_delay_test_phase <= DELAY_TEST_READY_MEASURE))
    {
        return 3U;
    }
#endif
    if (s_competition_run_id == 0U)
    {
        return 1U;
    }
    if ((s_competition_phase == COMPETITION_WAIT_POWER) ||
        (s_competition_phase == COMPETITION_WAIT_VISION))
    {
        return 2U;
    }
    if (s_competition_phase == COMPETITION_READY)
    {
        return 4U;
    }
    if (s_competition_phase == COMPETITION_COMPLETE)
    {
        return 6U;
    }
    if ((s_competition_phase == COMPETITION_Q2_POSITIVE) ||
        (s_competition_phase == COMPETITION_Q2_NEGATIVE))
    {
        return 3U;
    }
    if (s_competition_phase == COMPETITION_RUNNING)
    {
        return s_competition_question_id == 1U ? 5U : 5U;
    }
    return 8U;
}

int32_t gimbal_app_target_position_um(void)
{
    return s_competition_target_um;
}

int32_t gimbal_app_command_pipe_angle_mdeg(void)
{
    return s_command_pipe_angle_mdeg;
}

uint16_t gimbal_app_vision_age_ms(uint32_t now_ms)
{
    uint32_t age;
    if (s_vision_received == 0U)
    {
        return 0xFFFFU;
    }
    age = now_ms - s_vision.receive_time_ms;
    return age > 0xFFFFU ? 0xFFFFU : (uint16_t)age;
}

uint8_t gimbal_app_motor_enabled(void)
{
    return ((s_power_output_enabled != 0U) &&
            ((s_state == GIMBAL_STATE_COMPETITION)
#if GIMBAL_ENABLE_DELAY_TEST
             || (s_state == GIMBAL_STATE_DELAY_TEST)
#endif
            )) ? 1U : 0U;
}

uint8_t gimbal_app_stop_latched(void)
{
    return ((s_fault != GIMBAL_FAULT_NONE) ||
            (s_competition_phase == COMPETITION_FAULT)) ? 1U : 0U;
}

void gimbal_app_force_ti_safe_stop(
    uint32_t now_ms, GimbalTiStopMode stop_mode)
{
    GimbalFault stop_fault = GIMBAL_FAULT_NONE;
    if (stop_mode == GIMBAL_TI_STOP_LINK_FAULT)
    {
        stop_fault = GIMBAL_FAULT_TI_LINK;
    }
    else if (stop_mode == GIMBAL_TI_STOP_PRESERVE_FAULT)
    {
        stop_fault = s_fault;
    }
    enter_stopped_state(
        stop_fault, now_ms);
    s_competition_run_id = 0U;
    s_competition_question_id = 0U;
    s_competition_start_ms = 0U;
    s_target_settle_start_ms = 0U;
    s_q2_breakaway_deadline_ms = 0U;
    s_q2_final_capture_latched = 0U;
    competition_q2_stall_reset();
    s_competition_target_um = 0;
}
