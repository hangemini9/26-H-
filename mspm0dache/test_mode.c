#include "test_mode.h"

#include <float.h>

#include "chassis.h"
#include "chassis_config.h"
#include "line_sensor.h"
#include "motor.h"
#include "platform.h"
#include "question_timer.h"
#include "ti_msp_dl_config.h"

#define FW_BUILD_ID               2026080104U
#define BUTTON_DEBOUNCE_MS                40U
#define BUTTON_COUNT                       8U
#define BUTTON_EVENT_QUEUE_SIZE           16U
#define BUTTON_ACTION_PRESS                1U
#define BUTTON_ACTION_RELEASE              2U
#define BUTTON6_MASK                 (1U << 5)
#define BUTTON7_MASK                 (1U << 6)
#define BUTTON8_MASK                 (1U << 7)
#define START_DELAY_MS                  3000U
#define RUN_DURATION_MS                20000U
#define BLUETOOTH_HEARTBEAT_TIMEOUT_MS  2000U

#define START_SOURCE_NONE                   0U
#define START_SOURCE_BUTTON                 1U
#define START_SOURCE_BLUETOOTH              2U
#define START_SOURCE_MC02                   3U
#define MC02_IDLE_DISARMED_STATE             1U

#define LINE_SENSOR_VALID                    0
#define LINE_SENSOR_HOLDING                  1
#define LINE_SENSOR_RECOVERING               2
#define LINE_SENSOR_HARD_WAIT                3
#define LINE_SENSOR_FAULTED                  4

ChassisTestControl g_test = {
    .mode = TEST_IDLE,
    .arm = 0,
    .status = TEST_STATUS_IDLE,
    .build_id = FW_BUILD_ID,
    .duration_ms = RUN_DURATION_MS,
    .target_rpm = 40.0f,
    .line = {
        .line_lost = 1U,
        .base_rpm = LINE_FOLLOW_DEFAULT_BASE_RPM,
        .steering_kp = LINE_FOLLOW_DEFAULT_STEERING_KP,
        .steering_kd = LINE_FOLLOW_DEFAULT_STEERING_KD,
    },
    .left = {
        .kp = MOTOR_PID_DEFAULT_KP,
        .ki = MOTOR_PID_DEFAULT_KI,
        .kd = MOTOR_PID_DEFAULT_KD,
    },
    .right = {
        .kp = MOTOR_PID_DEFAULT_KP,
        .ki = MOTOR_PID_DEFAULT_KI,
        .kd = MOTOR_PID_DEFAULT_KD,
    },
};

#if TEST_MODE
typedef struct {
    uint8_t id;
    uint8_t action;
} ButtonEvent;

static int32_t s_last_mode;
static uint32_t s_start_ms;
static uint32_t s_run_duration_ms;
static uint32_t s_encoder_watch_left_ms;
static uint32_t s_encoder_watch_right_ms;
static int64_t s_encoder_watch_left_count;
static int64_t s_encoder_watch_right_count;
static int8_t s_expected_left_direction;
static int8_t s_expected_right_direction;
static uint8_t s_run_active;
static uint32_t s_encoder_monitor_started_mask;
static uint32_t s_encoder_verified_mask;
static uint8_t s_button_raw_mask;
static uint8_t s_button_stable_mask;
static uint8_t s_button_can_trigger_mask;
static uint8_t s_button_panel_initialized;
static uint8_t s_emergency_stop_latched;
static uint8_t s_power_enabled;
static ButtonEvent s_button_events[BUTTON_EVENT_QUEUE_SIZE];
static uint8_t s_button_event_head;
static uint8_t s_button_event_tail;
static uint8_t s_start_pending;
static uint8_t s_start_source;
static int32_t s_start_mode;
static uint32_t s_start_duration_ms;
static uint32_t s_button_raw_change_ms[BUTTON_COUNT];
static uint32_t s_start_request_ms;
static uint32_t s_bluetooth_keepalive_ms;
static uint32_t s_start_sensor_check_ms;
static uint32_t s_start_sensor_sequence;
static uint8_t s_start_sensor_check_active;
static uint8_t s_start_sensor_valid_samples;
static uint32_t s_line_last_sample_ms;
static uint32_t s_line_fault_start_ms;
static uint32_t s_line_last_sample_sequence;
static uint8_t s_line_fault_pending;
static uint8_t s_line_lap_active;
static uint8_t s_turn_count;
static uint8_t s_supervisor_question_id;
static int64_t s_lap_start_left_count;
static int64_t s_lap_start_right_count;
static int64_t s_finish_left_count;
static int64_t s_finish_right_count;
static int64_t s_reverse_left_count;
static int64_t s_reverse_right_count;
static uint32_t s_finish_phase_start_ms;
static uint32_t s_finish_sample_sequence;
static uint32_t s_finish_marker_last_seen_ms[LINE_SENSOR_CHANNEL_COUNT];
static uint8_t s_finish_centered_samples;
static uint8_t s_finish_marker_confirm_samples;
static float s_line_command_left_rpm;
static float s_line_command_right_rpm;
static float s_line_common_command_rpm;
static float s_line_steering_command_rpm;
static uint8_t s_line_common_motion_phase;
static int32_t s_last_valid_line_position_x1000;
static float s_filtered_line_position;
static float s_previous_line_position;

static void local_operator_stop(TestStatus status);
#endif

static void update_wheel_telemetry(
    ChassisTestWheel *watch, Motor_ID id)
{
    watch->encoder_count = Motor_GetTotalCount(id);
    watch->rpm = Motor_GetSpeedRPM(id);
    watch->pwm = Motor_GetAppliedPWM(id);

    if (g_test.status == TEST_STATUS_RUNNING &&
        (g_test.mode == TEST_CLOSED_LOOP_RPM ||
            g_test.mode == TEST_LINE_FOLLOW)) {
        float target = g_test.target_rpm;
        if (g_test.mode == TEST_LINE_FOLLOW) {
            target = id == MOTOR_LEFT ?
                g_test.line.left_target_rpm :
                g_test.line.right_target_rpm;
        }
        watch->error_rpm = target - watch->rpm;
        watch->integral = Motor_GetPIDIntegral(id);
        watch->pid_output = Motor_GetPIDOutput(id);
    }
}

static void update_telemetry(void)
{
    const LineSensorData *line = LineSensor_GetData();
    const QuestionTimerState *timer = QuestionTimer_GetState();
    g_test.build_id = FW_BUILD_ID;
    g_test.motor_armed = Motor_IsArmed() ? 1U : 0U;
    g_test.left_invalid_transitions =
        Motor_GetLeftEncoderInvalidTransitions();
    update_wheel_telemetry(&g_test.left, MOTOR_LEFT);
    update_wheel_telemetry(&g_test.right, MOTOR_RIGHT);
    g_test.line.connected = line->connected;
    g_test.line.sample_sequence = line->sample_sequence;
    g_test.line.error_count = line->error_count;
    g_test.line.state = line->state;
    g_test.line.active_count = line->active_count;
    g_test.line.line_lost = line->line_lost;
    g_test.line.position_x1000 = line->position_x1000;
    for (uint32_t i = 0U; i < LINE_SENSOR_CHANNEL_COUNT; i++) {
        g_test.line.analog[i] = line->analog[i];
        g_test.line.threshold[i] = line->threshold[i];
    }
    g_test.timer.running = timer->running;
    g_test.timer.question_id = timer->question_id;
    g_test.timer.power_enabled = timer->power_enabled;
    g_test.timer.elapsed_ms = timer->elapsed_ms;
    g_test.timer.elapsed_seconds = timer->elapsed_seconds;
    g_test.timer.oled_connected = timer->oled_connected;
    g_test.timer.oled_address = timer->oled_address;
    g_test.timer.oled_error_count = timer->oled_error_count;
    g_test.timer.oled_requested_seconds =
        timer->oled_requested_seconds;
    g_test.timer.oled_displayed_seconds =
        timer->oled_displayed_seconds;
    g_test.timer.oled_render_count = timer->oled_render_count;
#if TEST_MODE
    g_test.encoder_verified_mask = s_encoder_verified_mask;
#endif
}

#if TEST_MODE
static float abs_float(float value)
{
    return value < 0.0f ? -value : value;
}

static int float_is_finite(float value)
{
    return value == value && value <= FLT_MAX && value >= -FLT_MAX;
}

static int8_t float_sign(float value)
{
    if (value > 0.0f) {
        return 1;
    }
    if (value < 0.0f) {
        return -1;
    }
    return 0;
}

static int valid_nonzero_rpm(float value)
{
    return float_is_finite(value) && abs_float(value) >= 1.0f &&
        abs_float(value) <= MOTOR_MAX_RPM;
}

static int valid_pid_gains(const ChassisTestWheel *wheel)
{
    return float_is_finite(wheel->kp) &&
        float_is_finite(wheel->ki) &&
        float_is_finite(wheel->kd) &&
        wheel->kp >= 0.0f && wheel->ki >= 0.0f && wheel->kd >= 0.0f;
}

static int valid_line_follow_params(void)
{
    return float_is_finite(g_test.line.base_rpm) &&
        float_is_finite(g_test.line.steering_kp) &&
        float_is_finite(g_test.line.steering_kd) &&
        g_test.line.base_rpm >= LINE_FOLLOW_MIN_BASE_RPM &&
        g_test.line.base_rpm <= LINE_FOLLOW_MAX_BASE_RPM &&
        g_test.line.steering_kp >= 0.0f &&
        g_test.line.steering_kp <= LINE_FOLLOW_MAX_STEERING_KP &&
        g_test.line.steering_kd >= 0.0f &&
        g_test.line.steering_kd <= LINE_FOLLOW_MAX_STEERING_KD;
}

static float min_float(float a, float b)
{
    return a < b ? a : b;
}

static uint8_t popcount_u8(uint8_t value)
{
    uint8_t count = 0U;
    while (value != 0U) {
        count = (uint8_t) (count + (value & 1U));
        value >>= 1U;
    }
    return count;
}

static float competition_base_rpm(void)
{
    if (s_supervisor_question_id == 1U) {
        return LINE_FOLLOW_Q1_BASE_RPM;
    }
    if (s_supervisor_question_id == 3U) {
        return LINE_FOLLOW_Q3_CRUISE_RPM;
    }
    if (s_supervisor_question_id == 4U ||
        s_supervisor_question_id == 5U) {
        return LINE_FOLLOW_Q45_BASE_RPM;
    }
    return g_test.line.base_rpm;
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

static uint8_t read_button_mask(void)
{
    uint8_t mask = 0U;
    if (DL_GPIO_readPins(GPIO_BUTTON_PANEL_A_PORT,
            GPIO_BUTTON_PANEL_A_BUTTON1_PIN) == 0U) {
        mask |= 1U << 0;
    }
    if (DL_GPIO_readPins(GPIO_BUTTON_PANEL_A_PORT,
            GPIO_BUTTON_PANEL_A_BUTTON2_PIN) == 0U) {
        mask |= 1U << 1;
    }
    if (DL_GPIO_readPins(GPIO_BUTTON_PANEL_B_PORT,
            GPIO_BUTTON_PANEL_B_BUTTON3_PIN) == 0U) {
        mask |= 1U << 2;
    }
    if (DL_GPIO_readPins(GPIO_BUTTON_PANEL_B_PORT,
            GPIO_BUTTON_PANEL_B_BUTTON4_PIN) == 0U) {
        mask |= 1U << 3;
    }
    if (DL_GPIO_readPins(GPIO_BUTTON_PANEL_A_PORT,
            GPIO_BUTTON_PANEL_A_BUTTON5_PIN) == 0U) {
        mask |= 1U << 4;
    }
    if (DL_GPIO_readPins(GPIO_BUTTON_PANEL_A_PORT,
            GPIO_BUTTON_PANEL_A_BUTTON6_PIN) == 0U) {
        mask |= BUTTON6_MASK;
    }
    if (DL_GPIO_readPins(GPIO_BUTTON_PANEL_B_PORT,
            GPIO_BUTTON_PANEL_B_BUTTON7_PIN) == 0U) {
        mask |= BUTTON7_MASK;
    }
    if (DL_GPIO_readPins(GPIO_BUTTON_PANEL_B_PORT,
            GPIO_BUTTON_PANEL_B_BUTTON8_PIN) == 0U) {
        mask |= BUTTON8_MASK;
    }
    return mask;
}

static void queue_button_event(uint8_t button_id, uint8_t action)
{
    uint8_t next = (uint8_t) ((s_button_event_head + 1U) %
        BUTTON_EVENT_QUEUE_SIZE);
    if (next == s_button_event_tail) {
        g_test.button_event_drops++;
        return;
    }
    s_button_events[s_button_event_head].id = button_id;
    s_button_events[s_button_event_head].action = action;
    s_button_event_head = next;
}

static void set_power_enabled(uint8_t enabled)
{
    s_power_enabled = enabled != 0U ? 1U : 0U;
    if (s_power_enabled == 0U) {
        Motor_Disarm();
        g_test.motor_armed = 0U;
        g_test.left.pwm = 0;
        g_test.right.pwm = 0;
    }
    QuestionTimer_SetPowerEnabled(s_power_enabled);
}

static void cancel_pending_start(void)
{
    s_start_pending = 0U;
    s_start_sensor_check_active = 0U;
    s_start_sensor_valid_samples = 0U;
    s_start_source = START_SOURCE_NONE;
    s_start_mode = TEST_IDLE;
    s_start_duration_ms = 0U;
    g_test.start_delay_ms = 0U;
    if (g_test.status == TEST_STATUS_START_DELAY) {
        g_test.status = TEST_STATUS_IDLE;
    }
}

static int begin_start_delay(
    uint8_t source, int32_t mode, uint32_t duration_ms)
{
    uint32_t max_duration = mode == TEST_LINE_FOLLOW ?
        LINE_FOLLOW_MAX_DURATION_MS : TEST_MAX_DURATION_MS;
    if ((source != START_SOURCE_BUTTON &&
            source != START_SOURCE_BLUETOOTH &&
            source != START_SOURCE_MC02) ||
        (mode != TEST_CLOSED_LOOP_RPM &&
            mode != TEST_LINE_FOLLOW) ||
        (source == START_SOURCE_BUTTON &&
            mode != TEST_LINE_FOLLOW) ||
        duration_ms < TEST_ENCODER_MIN_RUN_DURATION_MS ||
        duration_ms > max_duration ||
        s_power_enabled == 0U ||
        s_run_active != 0U || s_start_pending != 0U ||
        g_test.mode != TEST_IDLE) {
        return 0;
    }

    uint32_t now = Platform_Millis();
    s_start_pending = 1U;
    s_start_source = source;
    s_start_mode = mode;
    s_start_duration_ms = duration_ms;
    s_start_request_ms = now;
    s_bluetooth_keepalive_ms = now;
    s_start_sensor_check_active = 0U;
    s_start_sensor_valid_samples = 0U;
    g_test.status = TEST_STATUS_START_DELAY;
    g_test.start_delay_ms = START_DELAY_MS;
    return 1;
}

static void update_user_buttons(void)
{
    uint32_t now = Platform_Millis();
    uint8_t raw_mask = read_button_mask();

    /*
     * K7/K8 are fail-safe: a new raw low removes motor authority in this
     * 10 ms control cycle. Their outward button events are still debounced
     * below. K8 latches; K7 may clear that latch only after both the K7
     * press is debounced and K8 is released.
     */
    if ((raw_mask & BUTTON7_MASK) != 0U &&
        (s_button_raw_mask & BUTTON7_MASK) == 0U) {
        set_power_enabled(0U);
        local_operator_stop(
            s_emergency_stop_latched != 0U ?
                TEST_STATUS_EMERGENCY_STOP :
                TEST_STATUS_IDLE);
    }
    if ((raw_mask & BUTTON8_MASK) != 0U &&
        s_emergency_stop_latched == 0U) {
        s_emergency_stop_latched = 1U;
        set_power_enabled(0U);
        local_operator_stop(TEST_STATUS_EMERGENCY_STOP);
    }

    for (uint8_t index = 0U; index < BUTTON_COUNT; index++) {
        uint8_t bit = (uint8_t) (1U << index);
        uint8_t raw_pressed = (raw_mask & bit) != 0U ? 1U : 0U;
        uint8_t previous_raw =
            (s_button_raw_mask & bit) != 0U ? 1U : 0U;
        uint8_t stable_pressed =
            (s_button_stable_mask & bit) != 0U ? 1U : 0U;

        if (raw_pressed != previous_raw) {
            if (raw_pressed != 0U) {
                s_button_raw_mask |= bit;
            } else {
                s_button_raw_mask &= (uint8_t) ~bit;
            }
            s_button_raw_change_ms[index] = now;
        }
        if (raw_pressed == stable_pressed ||
            (uint32_t) (now - s_button_raw_change_ms[index]) <
                BUTTON_DEBOUNCE_MS) {
            continue;
        }

        if (raw_pressed != 0U) {
            s_button_stable_mask |= bit;
        } else {
            s_button_stable_mask &= (uint8_t) ~bit;
        }
        g_test.button_mask = s_button_stable_mask;
        g_test.last_button_id = (uint32_t) index + 1U;
        queue_button_event((uint8_t) (index + 1U),
            raw_pressed != 0U ?
                BUTTON_ACTION_PRESS : BUTTON_ACTION_RELEASE);

        if (raw_pressed == 0U) {
            s_button_can_trigger_mask |= bit;
        } else if ((s_button_can_trigger_mask & bit) != 0U) {
            s_button_can_trigger_mask &= (uint8_t) ~bit;
            if (index == 7U) {
                if (s_emergency_stop_latched == 0U) {
                    s_emergency_stop_latched = 1U;
                    set_power_enabled(0U);
                    local_operator_stop(TEST_STATUS_EMERGENCY_STOP);
                }
            } else if (index == 6U) {
                set_power_enabled(0U);
                if ((s_button_stable_mask & BUTTON8_MASK) == 0U) {
                    s_emergency_stop_latched = 0U;
                }
                local_operator_stop(
                    s_emergency_stop_latched != 0U ?
                        TEST_STATUS_EMERGENCY_STOP :
                        TEST_STATUS_IDLE);
            } else if (index == 5U) {
                if (s_power_enabled != 0U) {
                    set_power_enabled(0U);
                    local_operator_stop(TEST_STATUS_IDLE);
                } else if (s_emergency_stop_latched == 0U &&
                    s_run_active == 0U &&
                    s_start_pending == 0U &&
                    g_test.mc02.online != 0U &&
                    g_test.mc02.state ==
                        MC02_IDLE_DISARMED_STATE &&
                    g_test.mode == TEST_IDLE) {
                    set_power_enabled(1U);
                }
            }
#if STANDALONE_KEY1_START
            else if (index == 0U && s_run_active == 0U &&
                s_start_pending == 0U &&
                g_test.mode == TEST_IDLE) {
                (void) begin_start_delay(
                    START_SOURCE_BUTTON, TEST_LINE_FOLLOW,
                    LINE_FOLLOW_LAP_DURATION_MS);
            }
#endif
        }
    }

    if (s_start_pending == 0U) {
        return;
    }
    if (g_test.mode != TEST_IDLE) {
        cancel_pending_start();
        return;
    }
    if (s_start_source == START_SOURCE_BLUETOOTH &&
        (uint32_t) (now - s_bluetooth_keepalive_ms) >
            BLUETOOTH_HEARTBEAT_TIMEOUT_MS) {
        cancel_pending_start();
        return;
    }

    uint32_t delay_elapsed = (uint32_t) (now - s_start_request_ms);
    if (delay_elapsed >= START_DELAY_MS) {
        const LineSensorData *line = LineSensor_GetData();
        if (s_start_sensor_check_active == 0U) {
            /*
             * Keep the motor disarmed after the countdown until samples
             * acquired after this point prove that the line sensor is live.
             * A previously cached valid sample is not sufficient to start.
             */
            s_start_sensor_check_active = 1U;
            s_start_sensor_valid_samples = 0U;
            s_start_sensor_check_ms = now;
            s_start_sensor_sequence = line->sample_sequence;
            g_test.start_delay_ms = 0U;
            return;
        }
        if ((uint32_t) (now - s_start_sensor_check_ms) >=
            LINE_FOLLOW_START_SENSOR_TIMEOUT_MS) {
            cancel_pending_start();
            g_test.status = TEST_STATUS_LINE_SENSOR_FAULT;
            return;
        }
        if (line->sample_sequence == s_start_sensor_sequence) {
            return;
        }
        s_start_sensor_sequence = line->sample_sequence;
        if (line->connected != 0U && line->line_lost == 0U) {
            if (s_start_sensor_valid_samples < UINT8_MAX) {
                s_start_sensor_valid_samples++;
            }
        } else {
            s_start_sensor_valid_samples = 0U;
        }
        if (s_start_sensor_valid_samples <
            LINE_FOLLOW_START_VALID_SAMPLES) {
            return;
        }
        s_start_pending = 0U;
        s_start_sensor_check_active = 0U;
        g_test.start_delay_ms = 0U;
        g_test.duration_ms = s_start_duration_ms;
        if (s_start_source == START_SOURCE_BLUETOOTH) {
            s_bluetooth_keepalive_ms = now;
        }
        g_test.arm = 1;
        g_test.mode = s_start_mode;
    } else {
        g_test.start_delay_ms = START_DELAY_MS - delay_elapsed;
    }
}

static uint32_t limited_duration_ms(int32_t mode)
{
    uint32_t duration = g_test.duration_ms;
    if (duration == 0U) {
        duration = TEST_DEFAULT_DURATION_MS;
    }
    uint32_t maximum = mode == TEST_LINE_FOLLOW ?
        LINE_FOLLOW_MAX_DURATION_MS : TEST_MAX_DURATION_MS;
    if (duration > maximum) {
        duration = maximum;
    }
    g_test.duration_ms = duration;
    return duration;
}

static float limited_target_rpm(void)
{
    if (!float_is_finite(g_test.target_rpm)) {
        return 0.0f;
    }
    float rpm = abs_float(g_test.target_rpm);
    if (rpm > MOTOR_MAX_RPM) {
        rpm = MOTOR_MAX_RPM;
    }
    return rpm;
}

static int is_motion_mode(int32_t mode)
{
    return mode >= TEST_RIGHT_OPEN_LOOP &&
        mode <= TEST_LINE_FOLLOW;
}

static int is_closed_loop_mode(int32_t mode)
{
    return mode == TEST_CLOSED_LOOP_RPM ||
        mode == TEST_LINE_FOLLOW;
}

static void clear_run_telemetry(void)
{
    g_test.elapsed_ms = 0U;
    g_test.motor_armed = 0U;
    g_test.output_fault_mask = 0U;
    g_test.encoder_fault_mask = 0U;
    g_test.encoder_verified_mask = 0U;
    g_test.left.error_rpm = 0.0f;
    g_test.right.error_rpm = 0.0f;
    g_test.left.integral = 0.0f;
    g_test.right.integral = 0.0f;
    g_test.left.pid_output = 0.0f;
    g_test.right.pid_output = 0.0f;
    g_test.line.correction_rpm = 0.0f;
    g_test.line.left_target_rpm = 0.0f;
    g_test.line.right_target_rpm = 0.0f;
    g_test.line.marker_recent_state = 0U;
    g_test.line.marker_recent_count = 0U;
    s_line_common_motion_phase = TEST_COMMON_MOTION_STEADY;
}

static void reset_encoder_watchdog(void)
{
    uint32_t now = Platform_Millis();
    s_encoder_watch_left_ms = now;
    s_encoder_watch_right_ms = now;
    s_encoder_watch_left_count = Motor_GetTotalCount(MOTOR_LEFT);
    s_encoder_watch_right_count = Motor_GetTotalCount(MOTOR_RIGHT);
    s_encoder_monitor_started_mask = 0U;
    s_encoder_verified_mask = 0U;
}

static void finish_run(TestStatus status);

static float average_forward_distance_mm(
    int64_t start_left_count, int64_t start_right_count)
{
    float left_mm = Chassis_CountsToMm(MOTOR_LEFT,
        Motor_GetTotalCount(MOTOR_LEFT) - start_left_count);
    float right_mm = Chassis_CountsToMm(MOTOR_RIGHT,
        Motor_GetTotalCount(MOTOR_RIGHT) - start_right_count);
    return 0.5f * (left_mm + right_mm);
}

static void update_lap_turn_assist(float start_distance)
{
    float left_mm = Chassis_CountsToMm(MOTOR_LEFT,
        Motor_GetTotalCount(MOTOR_LEFT) - s_lap_start_left_count);
    float right_mm = Chassis_CountsToMm(MOTOR_RIGHT,
        Motor_GetTotalCount(MOTOR_RIGHT) - s_lap_start_right_count);
    float yaw_deg = (right_mm - left_mm) /
        CHASSIS_DRIVE_TRACK_MM * (180.0f / CHASSIS_PI);
    float abs_yaw_deg = abs_float(yaw_deg);

    g_test.line.lap_yaw_deg = yaw_deg;
    if (s_turn_count < 1U &&
        start_distance >= LINE_TURN1_MIN_DISTANCE_MM &&
        abs_yaw_deg >= LINE_TURN1_MIN_ABS_YAW_DEG) {
        s_turn_count = 1U;
    }
    if (s_turn_count < 2U &&
        start_distance >= LINE_TURN2_MIN_DISTANCE_MM &&
        abs_yaw_deg >= LINE_TURN2_MIN_ABS_YAW_DEG) {
        s_turn_count = 2U;
    }
    g_test.line.turn_count = s_turn_count;
}

static float slew_line_target(float current, float target)
{
    float up_step = LINE_FOLLOW_ACCEL_SLEW_RPM_PER_S *
        ((float) MOTOR_CTRL_PERIOD_MS / 1000.0f);
    float down_step = LINE_FOLLOW_DECEL_SLEW_RPM_PER_S *
        ((float) MOTOR_CTRL_PERIOD_MS / 1000.0f);
    if (target > current + up_step) {
        return current + up_step;
    }
    if (target < current - down_step) {
        return current - down_step;
    }
    return target;
}

static float slew_line_target_with_rates(
    float current, float target, float up_rate, float down_rate)
{
    float up_step = up_rate *
        ((float) MOTOR_CTRL_PERIOD_MS / 1000.0f);
    float down_step = down_rate *
        ((float) MOTOR_CTRL_PERIOD_MS / 1000.0f);
    if (target > current + up_step) {
        return current + up_step;
    }
    if (target < current - down_step) {
        return current - down_step;
    }
    return target;
}

static int use_gentle_motion_profile(void)
{
    return s_start_source == START_SOURCE_MC02 &&
        s_supervisor_question_id >= 3U &&
        s_supervisor_question_id <= 5U;
}

static void apply_line_targets(
    float left_target, float right_target, float correction)
{
    if (use_gentle_motion_profile()) {
        float common_accel_rpm_s =
            s_supervisor_question_id == 3U ?
            LINE_FOLLOW_Q3_ACCEL_RPM_PER_S :
            LINE_FOLLOW_Q45_ACCEL_RPM_PER_S;
        float common_decel_rpm_s =
            s_supervisor_question_id == 3U ?
            LINE_FOLLOW_Q3_DECEL_RPM_PER_S :
            LINE_FOLLOW_Q45_DECEL_RPM_PER_S;
        float requested_common =
            (left_target + right_target) * 0.5f;
        float requested_steering =
            (left_target - right_target) * 0.5f;
        float previous_common = s_line_common_command_rpm;
        s_line_common_command_rpm = slew_line_target_with_rates(
            s_line_common_command_rpm,
            requested_common,
            common_accel_rpm_s,
            common_decel_rpm_s);
        if (s_line_common_command_rpm > previous_common + 0.01f) {
            s_line_common_motion_phase =
                TEST_COMMON_MOTION_ACCELERATING;
        } else if (s_line_common_command_rpm < previous_common - 0.01f) {
            s_line_common_motion_phase =
                TEST_COMMON_MOTION_DECELERATING;
        } else {
            s_line_common_motion_phase = TEST_COMMON_MOTION_STEADY;
        }
        s_line_steering_command_rpm = slew_line_target_with_rates(
            s_line_steering_command_rpm,
            requested_steering,
            LINE_FOLLOW_STEERING_SLEW_RPM_PER_S,
            LINE_FOLLOW_STEERING_SLEW_RPM_PER_S);
        s_line_command_left_rpm =
            s_line_common_command_rpm + s_line_steering_command_rpm;
        s_line_command_right_rpm =
            s_line_common_command_rpm - s_line_steering_command_rpm;
        correction = s_line_steering_command_rpm;
    } else {
        s_line_common_motion_phase = TEST_COMMON_MOTION_STEADY;
        s_line_command_left_rpm =
            slew_line_target(s_line_command_left_rpm, left_target);
        s_line_command_right_rpm =
            slew_line_target(s_line_command_right_rpm, right_target);
        s_line_common_command_rpm =
            (s_line_command_left_rpm + s_line_command_right_rpm) * 0.5f;
        s_line_steering_command_rpm =
            (s_line_command_left_rpm - s_line_command_right_rpm) * 0.5f;
    }
    g_test.line.correction_rpm = correction;
    g_test.line.left_target_rpm = s_line_command_left_rpm;
    g_test.line.right_target_rpm = s_line_command_right_rpm;
    Motor_SetTargetRPM(MOTOR_LEFT, s_line_command_left_rpm);
    Motor_SetTargetRPM(MOTOR_RIGHT, s_line_command_right_rpm);
}

static void set_line_zero_targets(void)
{
    s_line_command_left_rpm = 0.0f;
    s_line_command_right_rpm = 0.0f;
    s_line_common_command_rpm = 0.0f;
    s_line_steering_command_rpm = 0.0f;
    s_line_common_motion_phase = TEST_COMMON_MOTION_STEADY;
    g_test.line.correction_rpm = 0.0f;
    g_test.line.left_target_rpm = 0.0f;
    g_test.line.right_target_rpm = 0.0f;
    Motor_SetTargetRPM(MOTOR_LEFT, 0.0f);
    Motor_SetTargetRPM(MOTOR_RIGHT, 0.0f);
}

static void set_line_reverse_targets(void)
{
    apply_line_targets(
        -LINE_FINISH_REVERSE_RPM,
        -LINE_FINISH_REVERSE_RPM,
        0.0f);
}

static void set_line_straight_targets(float base_rpm)
{
    float position =
        (float) LineSensor_GetData()->position_x1000 / 1000.0f;
    s_filtered_line_position = position;
    s_previous_line_position = position;
    apply_line_targets(base_rpm, base_rpm, 0.0f);
}

static float line_follow_curve_base_rpm(
    float requested_base_rpm, float position)
{
    float curve_min_base_rpm =
        s_supervisor_question_id == 1U ?
            LINE_FOLLOW_Q1_CURVE_MIN_BASE_RPM :
            LINE_FOLLOW_CURVE_MIN_BASE_RPM;
    if (requested_base_rpm <= curve_min_base_rpm) {
        return requested_base_rpm;
    }
    float fraction = clamp_float(
        abs_float(position) / 3.5f, 0.0f, 1.0f);
    return requested_base_rpm -
        (requested_base_rpm - curve_min_base_rpm) *
            fraction;
}

static void set_line_follow_targets(float requested_base_rpm)
{
    float raw_position =
        (float) LineSensor_GetData()->position_x1000 / 1000.0f;
    s_filtered_line_position += LINE_FOLLOW_POSITION_FILTER_ALPHA *
        (raw_position - s_filtered_line_position);
    float position = s_filtered_line_position;
    float base_rpm =
        line_follow_curve_base_rpm(requested_base_rpm, position);
    float position_delta = position - s_previous_line_position;
    s_previous_line_position = position;
    float derivative_correction = clamp_float(
        g_test.line.steering_kd * position_delta,
        -LINE_FOLLOW_MAX_D_CORRECTION_RPM,
        LINE_FOLLOW_MAX_D_CORRECTION_RPM);
    float correction =
        g_test.line.steering_kp * position + derivative_correction;

    float correction_limit = LINE_FOLLOW_MAX_CORRECTION_RPM;
    correction_limit = min_float(correction_limit,
        base_rpm - LINE_FOLLOW_MIN_WHEEL_RPM);
    float max_wheel_rpm = s_supervisor_question_id == 1U ?
        LINE_FOLLOW_Q1_MAX_WHEEL_RPM :
        LINE_FOLLOW_MAX_WHEEL_RPM;
    correction_limit = min_float(
        correction_limit, max_wheel_rpm - base_rpm);
    correction = clamp_float(
        correction, -correction_limit, correction_limit);

    /*
     * Positive position means the line is toward vehicle-right. Speed up
     * the left wheel and slow the right wheel to steer right.
     */
    float left_target = base_rpm + correction;
    float right_target = base_rpm - correction;
    apply_line_targets(left_target, right_target, correction);
}

static void set_line_recovery_targets(void)
{
    float correction =
        s_last_valid_line_position_x1000 < 0 ?
            -LINE_FOLLOW_RECOVERY_CORRECTION_RPM :
            LINE_FOLLOW_RECOVERY_CORRECTION_RPM;
    if (s_last_valid_line_position_x1000 == 0) {
        correction = 0.0f;
    }
    apply_line_targets(
        LINE_FOLLOW_RECOVERY_BASE_RPM + correction,
        LINE_FOLLOW_RECOVERY_BASE_RPM - correction,
        correction);
}

static void initialize_line_lap(void)
{
    const LineSensorData *line = LineSensor_GetData();
    s_line_lap_active =
        (s_start_source == START_SOURCE_BUTTON ||
            s_start_source == START_SOURCE_MC02) ? 1U : 0U;
    s_turn_count = 0U;
    s_lap_start_left_count = Motor_GetTotalCount(MOTOR_LEFT);
    s_lap_start_right_count = Motor_GetTotalCount(MOTOR_RIGHT);
    s_finish_left_count = s_lap_start_left_count;
    s_finish_right_count = s_lap_start_right_count;
    s_reverse_left_count = s_lap_start_left_count;
    s_reverse_right_count = s_lap_start_right_count;
    s_finish_phase_start_ms = Platform_Millis();
    s_finish_sample_sequence = line->sample_sequence;
    s_finish_centered_samples = 0U;
    s_finish_marker_confirm_samples = 0U;
    for (uint8_t i = 0U; i < LINE_SENSOR_CHANNEL_COUNT; i++) {
        s_finish_marker_last_seen_ms[i] = 0U;
    }
    s_line_command_left_rpm = 0.0f;
    s_line_command_right_rpm = 0.0f;
    s_line_common_command_rpm = 0.0f;
    s_line_steering_command_rpm = 0.0f;
    s_line_common_motion_phase = TEST_COMMON_MOTION_STEADY;
    s_last_valid_line_position_x1000 = line->position_x1000;
    s_filtered_line_position =
        (float) line->position_x1000 / 1000.0f;
    s_previous_line_position = s_filtered_line_position;
    g_test.line.lap_distance_mm = 0.0f;
    g_test.line.finish_distance_mm = 0.0f;
    g_test.line.turn_count = 0U;
    g_test.line.lap_yaw_deg = 0.0f;
    g_test.line.marker_recent_state = 0U;
    g_test.line.marker_recent_count = 0U;
    g_test.line.lap_phase = s_line_lap_active != 0U ?
        LINE_LAP_PHASE_LEAVE_START : LINE_LAP_PHASE_IDLE;
}

static uint8_t recent_finish_marker_state(uint32_t now)
{
    uint8_t state = 0U;
    for (uint8_t i = 0U; i < LINE_SENSOR_CHANNEL_COUNT; i++) {
        uint32_t last_seen = s_finish_marker_last_seen_ms[i];
        if (last_seen != 0U &&
            (uint32_t) (now - last_seen) <=
                LINE_FINISH_MARKER_RECENT_MS) {
            state |= (uint8_t) (1U << i);
        }
    }
    return state;
}

static void begin_finish_reverse(uint32_t now)
{
    s_reverse_left_count = Motor_GetTotalCount(MOTOR_LEFT);
    s_reverse_right_count = Motor_GetTotalCount(MOTOR_RIGHT);
    s_finish_phase_start_ms = now;
    s_finish_sample_sequence = LineSensor_GetData()->sample_sequence;
    s_finish_marker_confirm_samples = 0U;
    for (uint8_t i = 0U; i < LINE_SENSOR_CHANNEL_COUNT; i++) {
        s_finish_marker_last_seen_ms[i] = 0U;
    }
    g_test.line.finish_distance_mm = 0.0f;
    g_test.line.marker_recent_state = 0U;
    g_test.line.marker_recent_count = 0U;
    s_expected_left_direction = -1;
    s_expected_right_direction = -1;
    reset_encoder_watchdog();
    g_test.line.lap_phase = LINE_LAP_PHASE_REVERSE_SEARCH;
}

static void begin_gentle_stop(uint32_t now)
{
    s_finish_phase_start_ms = now;
    s_expected_left_direction = 0;
    s_expected_right_direction = 0;
    g_test.line.lap_phase = LINE_LAP_PHASE_GENTLE_STOP;
}

static int update_line_lap(uint32_t now)
{
    if (s_line_lap_active == 0U) {
        return 0;
    }

    float start_distance = average_forward_distance_mm(
        s_lap_start_left_count, s_lap_start_right_count);
    g_test.line.lap_distance_mm = start_distance;

    /*
     * Internal Q3 is the printed A-to-B test. The A/B center lines are
     * separated by the specified 1500 mm straight, so encoder distance is
     * the route-state B crossing used for this question.
     */
    if (s_supervisor_question_id == 3U &&
        start_distance >= LINE_COURSE_STRAIGHT_MM) {
        if (g_test.line.lap_phase != LINE_LAP_PHASE_GENTLE_STOP) {
            begin_gentle_stop(now);
        }
    }

    update_lap_turn_assist(start_distance);
    if (g_test.line.lap_phase == LINE_LAP_PHASE_LEAVE_START) {
        if (start_distance >= LINE_FINISH_ARM_DISTANCE_MM) {
            g_test.line.lap_phase = LINE_LAP_PHASE_SEARCH_FINISH;
        }
    } else if (g_test.line.lap_phase ==
        LINE_LAP_PHASE_SEARCH_FINISH) {
        if ((s_turn_count >= 2U &&
                start_distance >=
                    LINE_FINISH_TURN_ASSIST_DISTANCE_MM) ||
            start_distance >= LINE_FINISH_MAX_LAP_DISTANCE_MM) {
            if (s_supervisor_question_id == 4U ||
                s_supervisor_question_id == 5U) {
                begin_gentle_stop(now);
            } else {
                s_finish_left_count = Motor_GetTotalCount(MOTOR_LEFT);
                s_finish_right_count = Motor_GetTotalCount(MOTOR_RIGHT);
                s_finish_sample_sequence =
                    LineSensor_GetData()->sample_sequence;
                s_finish_centered_samples = 0U;
                g_test.line.finish_distance_mm = 0.0f;
                g_test.line.lap_phase = LINE_LAP_PHASE_FORWARD_ALIGN;
            }
        }
    } else if (g_test.line.lap_phase ==
        LINE_LAP_PHASE_FORWARD_ALIGN) {
        const LineSensorData *line = LineSensor_GetData();
        float forward_align_distance = average_forward_distance_mm(
            s_finish_left_count, s_finish_right_count);
        g_test.line.finish_distance_mm = forward_align_distance;
        if (line->sample_sequence != s_finish_sample_sequence) {
            s_finish_sample_sequence = line->sample_sequence;
            if (line->active_count >= 1U &&
                line->active_count <= 2U &&
                line->position_x1000 >=
                    -LINE_FINISH_CENTER_MAX_POSITION_X1000 &&
                line->position_x1000 <=
                    LINE_FINISH_CENTER_MAX_POSITION_X1000) {
                if (s_finish_centered_samples < UINT8_MAX) {
                    s_finish_centered_samples++;
                }
            } else {
                s_finish_centered_samples = 0U;
            }
        }
        if (forward_align_distance >=
                LINE_FINISH_FORWARD_ALIGN_MIN_MM &&
            s_finish_centered_samples >=
                LINE_FINISH_CENTER_SAMPLES) {
            s_finish_phase_start_ms = now;
            s_expected_left_direction = 0;
            s_expected_right_direction = 0;
            set_line_zero_targets();
            g_test.line.lap_phase = LINE_LAP_PHASE_SETTLE;
        } else if (forward_align_distance >=
            LINE_FINISH_FORWARD_ALIGN_MAX_MM) {
            finish_run(TEST_STATUS_TIMEOUT);
            return 1;
        }
    } else if (g_test.line.lap_phase == LINE_LAP_PHASE_SETTLE) {
        uint32_t settle_ms = (uint32_t) (now - s_finish_phase_start_ms);
        if (settle_ms >= LINE_FINISH_SETTLE_MIN_MS &&
            abs_float(Motor_GetSpeedRPM(MOTOR_LEFT)) <=
                MOTOR_REVERSE_SAFE_RPM &&
            abs_float(Motor_GetSpeedRPM(MOTOR_RIGHT)) <=
                MOTOR_REVERSE_SAFE_RPM) {
            begin_finish_reverse(now);
        } else if (settle_ms >= LINE_FINISH_SETTLE_MAX_MS) {
            finish_run(TEST_STATUS_TIMEOUT);
            return 1;
        }
    } else if (g_test.line.lap_phase ==
        LINE_LAP_PHASE_REVERSE_SEARCH) {
        const LineSensorData *line = LineSensor_GetData();
        float reverse_distance = -average_forward_distance_mm(
            s_reverse_left_count, s_reverse_right_count);
        if (reverse_distance < 0.0f) {
            reverse_distance = 0.0f;
        }
        g_test.line.finish_distance_mm = reverse_distance;

        if (line->sample_sequence != s_finish_sample_sequence) {
            s_finish_sample_sequence = line->sample_sequence;
            for (uint8_t i = 0U;
                i < LINE_SENSOR_CHANNEL_COUNT; i++) {
                if ((line->state & (uint8_t) (1U << i)) != 0U) {
                    s_finish_marker_last_seen_ms[i] = now;
                }
            }
            uint8_t recent_state = recent_finish_marker_state(now);
            uint8_t recent_count = popcount_u8(recent_state);
            g_test.line.marker_recent_state = recent_state;
            g_test.line.marker_recent_count = recent_count;
            if (reverse_distance >= LINE_FINISH_REVERSE_ARM_MM &&
                line->active_count >=
                    LINE_FINISH_MARKER_MIN_ACTIVE_CHANNELS) {
                if (s_finish_marker_confirm_samples < UINT8_MAX) {
                    s_finish_marker_confirm_samples++;
                }
            } else {
                s_finish_marker_confirm_samples = 0U;
            }
            if (s_finish_marker_confirm_samples >=
                LINE_FINISH_MARKER_CONFIRM_SAMPLES) {
                g_test.line.lap_phase =
                    LINE_LAP_PHASE_MARKER_DETECTED;
                finish_run(TEST_STATUS_COMPLETE);
                return 1;
            }
        }
        if (reverse_distance >= LINE_FINISH_REVERSE_MAX_MM ||
            (uint32_t) (now - s_finish_phase_start_ms) >=
                LINE_FINISH_REVERSE_TIMEOUT_MS) {
            finish_run(TEST_STATUS_TIMEOUT);
            return 1;
        }
    } else if (g_test.line.lap_phase ==
        LINE_LAP_PHASE_GENTLE_STOP) {
        apply_line_targets(0.0f, 0.0f, 0.0f);
        if (abs_float(s_line_command_left_rpm) <= 0.1f &&
            abs_float(s_line_command_right_rpm) <= 0.1f &&
            abs_float(Motor_GetSpeedRPM(MOTOR_LEFT)) <=
                MOTOR_REVERSE_SAFE_RPM &&
            abs_float(Motor_GetSpeedRPM(MOTOR_RIGHT)) <=
                MOTOR_REVERSE_SAFE_RPM) {
            g_test.line.lap_phase = LINE_LAP_PHASE_COMPLETE;
            finish_run(TEST_STATUS_COMPLETE);
            return 1;
        }
        if ((uint32_t) (now - s_finish_phase_start_ms) >=
            LINE_FINISH_GENTLE_STOP_MAX_MS) {
            finish_run(TEST_STATUS_TIMEOUT);
            return 1;
        }
    }
    return 0;
}

static int line_follow_sensor_state(uint32_t now)
{
    const LineSensorData *line = LineSensor_GetData();
    if (line->sample_sequence != s_line_last_sample_sequence) {
        s_line_last_sample_sequence = line->sample_sequence;
        s_line_last_sample_ms = now;
    }

    int hard_invalid = line->connected == 0U ||
        (uint32_t) (now - s_line_last_sample_ms) >
            LINE_FOLLOW_SENSOR_GRACE_MS;
    int bounded_reverse_search =
        g_test.line.lap_phase == LINE_LAP_PHASE_REVERSE_SEARCH;
    if (!hard_invalid &&
        (line->line_lost == 0U || bounded_reverse_search)) {
        s_line_fault_pending = 0U;
        if (line->line_lost == 0U) {
            s_last_valid_line_position_x1000 = line->position_x1000;
        }
        return LINE_SENSOR_VALID;
    }

    if (s_line_fault_pending == 0U) {
        s_line_fault_pending = 1U;
        s_line_fault_start_ms = now;
    }

    uint32_t fault_elapsed =
        (uint32_t) (now - s_line_fault_start_ms);
    if (hard_invalid) {
        /* A disconnected or stale sensor is not eligible for recovery. */
        Motor_SetTargetRPM(MOTOR_LEFT, 0.0f);
        Motor_SetTargetRPM(MOTOR_RIGHT, 0.0f);
        s_line_command_left_rpm = 0.0f;
        s_line_command_right_rpm = 0.0f;
        g_test.line.correction_rpm = 0.0f;
        g_test.line.left_target_rpm = 0.0f;
        g_test.line.right_target_rpm = 0.0f;
        if (fault_elapsed < LINE_FOLLOW_SENSOR_GRACE_MS) {
            return LINE_SENSOR_HARD_WAIT;
        }
        finish_run(TEST_STATUS_LINE_SENSOR_FAULT);
        return LINE_SENSOR_FAULTED;
    }

    if (fault_elapsed >= LINE_FOLLOW_LINE_LOSS_RECOVERY_MS) {
        finish_run(TEST_STATUS_LINE_SENSOR_FAULT);
        return LINE_SENSOR_FAULTED;
    }
    if (fault_elapsed < LINE_FOLLOW_LINE_LOSS_HOLD_MS) {
        return LINE_SENSOR_HOLDING;
    }
    return LINE_SENSOR_RECOVERING;
}

static void finish_run(TestStatus status)
{
    /*
     * Capture the final controller state before disarm resets the PID.
     * Subsequent idle telemetry keeps these PID fields for the screenshot,
     * while encoder count/RPM and the now-zero applied PWM remain live.
     */
    update_telemetry();
    Motor_Disarm();
    QuestionTimer_Stop();
    s_run_active = 0U;
    s_line_common_motion_phase = TEST_COMMON_MOTION_STEADY;
    s_start_source = START_SOURCE_NONE;
    g_test.status = status;
    g_test.mode = TEST_IDLE;
    g_test.arm = 0;
    g_test.motor_armed = 0U;
    g_test.left.pwm = 0;
    g_test.right.pwm = 0;
    s_last_mode = TEST_IDLE;
}

static void local_operator_stop(TestStatus status)
{
    cancel_pending_start();
    finish_run(status);
}

static int prepare_motion_mode(int32_t mode)
{
    if (s_power_enabled == 0U) {
        Motor_Disarm();
        g_test.status = TEST_STATUS_NOT_ARMED;
        g_test.mode = TEST_IDLE;
        g_test.arm = 0;
        return 0;
    }
    if (s_emergency_stop_latched != 0U) {
        Motor_Disarm();
        g_test.status = TEST_STATUS_EMERGENCY_STOP;
        g_test.mode = TEST_IDLE;
        g_test.arm = 0;
        return 0;
    }
    if (g_test.arm != 1) {
        Motor_Disarm();
        g_test.status = TEST_STATUS_NOT_ARMED;
        g_test.mode = TEST_IDLE;
        g_test.arm = 0;
        return 0;
    }

    g_test.arm = 0;
    s_run_duration_ms = limited_duration_ms(mode);
    if (is_closed_loop_mode(mode)) {
        if (s_run_duration_ms < TEST_ENCODER_MIN_RUN_DURATION_MS ||
            !valid_pid_gains(&g_test.left) ||
            !valid_pid_gains(&g_test.right)) {
            finish_run(TEST_STATUS_BAD_PARAMETER);
            return 0;
        }
        if (mode == TEST_CLOSED_LOOP_RPM &&
            !valid_nonzero_rpm(g_test.target_rpm)) {
            finish_run(TEST_STATUS_BAD_PARAMETER);
            return 0;
        }
        if (mode == TEST_LINE_FOLLOW &&
            !valid_line_follow_params()) {
            finish_run(TEST_STATUS_BAD_PARAMETER);
            return 0;
        }
        if (mode == TEST_LINE_FOLLOW &&
            (LineSensor_GetData()->connected == 0U ||
                LineSensor_GetData()->line_lost != 0U)) {
            finish_run(TEST_STATUS_LINE_SENSOR_FAULT);
            return 0;
        }
        Motor_PID_SetParams(MOTOR_LEFT,
            g_test.left.kp, g_test.left.ki, g_test.left.kd);
        Motor_PID_SetParams(MOTOR_RIGHT,
            g_test.right.kp, g_test.right.ki, g_test.right.kd);
    }

    clear_run_telemetry();
    reset_encoder_watchdog();
    s_start_ms = Platform_Millis();
    s_expected_left_direction = 0;
    s_expected_right_direction = 0;
    s_line_fault_pending = 0U;
    s_line_last_sample_ms = s_start_ms;
    s_line_last_sample_sequence =
        LineSensor_GetData()->sample_sequence;
    if (mode == TEST_LINE_FOLLOW) {
        initialize_line_lap();
    } else {
        s_line_lap_active = 0U;
        g_test.line.lap_phase = LINE_LAP_PHASE_IDLE;
        g_test.line.turn_count = 0U;
        g_test.line.lap_yaw_deg = 0.0f;
        g_test.line.lap_distance_mm = 0.0f;
        g_test.line.finish_distance_mm = 0.0f;
        g_test.line.marker_recent_state = 0U;
        g_test.line.marker_recent_count = 0U;
    }
    g_test.status = TEST_STATUS_RUNNING;
    Motor_Arm();
    if (!Motor_IsArmed()) {
        finish_run(TEST_STATUS_BAD_PARAMETER);
        return 0;
    }
    s_run_active = 1U;
    if (s_start_source == START_SOURCE_MC02 &&
        s_supervisor_question_id >= 1U &&
        s_supervisor_question_id <= 5U) {
        QuestionTimer_Start(s_supervisor_question_id);
    }
    return 1;
}

static void enter_mode(int32_t mode)
{
    Motor_Disarm();
    s_run_active = 0U;
    clear_run_telemetry();

    if (mode == TEST_IDLE || mode == TEST_ENCODER_OBSERVE) {
        g_test.status = TEST_STATUS_IDLE;
        g_test.arm = 0;
        return;
    }

    if (mode == TEST_ENCODER_RESET) {
        Chassis_ResetOdom();
        g_test.status = TEST_STATUS_COMPLETE;
        g_test.mode = TEST_IDLE;
        g_test.arm = 0;
        return;
    }

    if (!is_motion_mode(mode)) {
        g_test.status = TEST_STATUS_BAD_PARAMETER;
        g_test.mode = TEST_IDLE;
        g_test.arm = 0;
        return;
    }

    if (!prepare_motion_mode(mode)) {
        return;
    }

    switch (mode) {
        case TEST_RIGHT_OPEN_LOOP:
            Motor_SetPWM(MOTOR_RIGHT, TEST_DEFAULT_OPEN_PWM);
            break;
        case TEST_LEFT_OPEN_LOOP:
            Motor_SetPWM(MOTOR_LEFT, TEST_DEFAULT_OPEN_PWM);
            break;
        case TEST_BOTH_OPEN_FORWARD:
            Motor_SetPWM(MOTOR_LEFT, TEST_DEFAULT_OPEN_PWM);
            Motor_SetPWM(MOTOR_RIGHT, TEST_DEFAULT_OPEN_PWM);
            break;
        case TEST_BOTH_OPEN_REVERSE:
            Motor_SetPWM(MOTOR_LEFT, -TEST_DEFAULT_OPEN_PWM);
            Motor_SetPWM(MOTOR_RIGHT, -TEST_DEFAULT_OPEN_PWM);
            break;
        case TEST_CLOSED_LOOP_RPM: {
            float rpm = limited_target_rpm();
            float signed_rpm = g_test.target_rpm < 0.0f ? -rpm : rpm;
            s_expected_left_direction = float_sign(signed_rpm);
            s_expected_right_direction = s_expected_left_direction;
            Motor_SetTargetRPM(MOTOR_LEFT, signed_rpm);
            Motor_SetTargetRPM(MOTOR_RIGHT, signed_rpm);
            break;
        }
        case TEST_LINE_FOLLOW:
            s_expected_left_direction = 1;
            s_expected_right_direction = 1;
            set_line_follow_targets(
                s_line_lap_active != 0U ?
                    LINE_FOLLOW_START_BASE_RPM :
                    g_test.line.base_rpm);
            break;
        default:
            finish_run(TEST_STATUS_BAD_PARAMETER);
            break;
    }
}

static int signed_progress_is_valid(
    int8_t expected_direction, int64_t current, int64_t previous)
{
    if (expected_direction > 0) {
        return current - previous >= TEST_ENCODER_MIN_PROGRESS_COUNTS;
    }
    if (expected_direction < 0) {
        return previous - current >= TEST_ENCODER_MIN_PROGRESS_COUNTS;
    }
    return 0;
}

static void supervise_wheel(
    uint32_t wheel_mask,
    int8_t expected_direction,
    int16_t actual_pwm,
    int64_t current_count,
    uint32_t now,
    uint32_t *watch_ms,
    int64_t *watch_count,
    uint32_t *output_fault_mask,
    uint32_t *encoder_fault_mask)
{
    int output_has_expected_sign =
        (expected_direction > 0 && actual_pwm > 0) ||
        (expected_direction < 0 && actual_pwm < 0);
    int output_has_wrong_sign =
        (expected_direction > 0 && actual_pwm < 0) ||
        (expected_direction < 0 && actual_pwm > 0);

    if (expected_direction == 0 || output_has_wrong_sign) {
        *output_fault_mask |= wheel_mask;
        return;
    }

    if ((s_encoder_monitor_started_mask & wheel_mask) == 0U) {
        if (!output_has_expected_sign) {
            *watch_ms = now;
            *watch_count = current_count;
            return;
        }
        s_encoder_monitor_started_mask |= wheel_mask;
        *watch_ms = now;
        *watch_count = current_count;
        return;
    }

    if ((uint32_t) (now - *watch_ms) <
        TEST_ENCODER_PROGRESS_WINDOW_MS) {
        return;
    }

    if (!signed_progress_is_valid(
            expected_direction, current_count, *watch_count)) {
        *encoder_fault_mask |= wheel_mask;
    } else {
        s_encoder_verified_mask |= wheel_mask;
        g_test.encoder_verified_mask = s_encoder_verified_mask;
    }
    *watch_ms = now;
    *watch_count = current_count;
}

static int closed_loop_supervision_failed(void)
{
    uint32_t now = Platform_Millis();
    uint32_t output_fault_mask = 0U;
    uint32_t encoder_fault_mask = 0U;

    supervise_wheel(
        1U,
        s_expected_left_direction,
        Motor_GetAppliedPWM(MOTOR_LEFT),
        Motor_GetTotalCount(MOTOR_LEFT),
        now,
        &s_encoder_watch_left_ms,
        &s_encoder_watch_left_count,
        &output_fault_mask,
        &encoder_fault_mask);
    supervise_wheel(
        2U,
        s_expected_right_direction,
        Motor_GetAppliedPWM(MOTOR_RIGHT),
        Motor_GetTotalCount(MOTOR_RIGHT),
        now,
        &s_encoder_watch_right_ms,
        &s_encoder_watch_right_count,
        &output_fault_mask,
        &encoder_fault_mask);

    if (output_fault_mask != 0U) {
        g_test.output_fault_mask = output_fault_mask;
        finish_run(TEST_STATUS_OUTPUT_FAULT);
        return 1;
    }
    if (encoder_fault_mask != 0U) {
        g_test.encoder_fault_mask = encoder_fault_mask;
        finish_run(TEST_STATUS_ENCODER_FAULT);
        return 1;
    }
    return 0;
}

static void update_running_mode(int32_t mode)
{
    if (s_run_active == 0U) {
        return;
    }

    uint32_t now = Platform_Millis();
    g_test.elapsed_ms = (uint32_t) (now - s_start_ms);
    g_test.status = TEST_STATUS_RUNNING;

    /*
     * Q2 is a chassis participation state, not a motion mode. Reassert
     * true-zero/disarm every control cycle until MC02 ends the question
     * with SAFE_STOP/CANCEL. The timeout is only a fail-safe bound.
     */
    if (s_supervisor_question_id == 2U) {
        Motor_Disarm();
        g_test.mode = TEST_IDLE;
        g_test.arm = 0;
        g_test.motor_armed = 0U;
        g_test.left.pwm = 0;
        g_test.right.pwm = 0;
        if (g_test.elapsed_ms >= s_run_duration_ms) {
            finish_run(TEST_STATUS_TIMEOUT);
        }
        return;
    }

    if (s_start_source == START_SOURCE_BLUETOOTH &&
        (uint32_t) (now - s_bluetooth_keepalive_ms) >
            BLUETOOTH_HEARTBEAT_TIMEOUT_MS) {
        finish_run(TEST_STATUS_TIMEOUT);
        return;
    }

    if (mode == TEST_CLOSED_LOOP_RPM) {
        if (!valid_nonzero_rpm(g_test.target_rpm) ||
            float_sign(g_test.target_rpm) != s_expected_left_direction) {
            finish_run(TEST_STATUS_BAD_PARAMETER);
            return;
        }

        float rpm = limited_target_rpm();
        float signed_rpm = g_test.target_rpm < 0.0f ? -rpm : rpm;
        Motor_SetTargetRPM(MOTOR_LEFT, signed_rpm);
        Motor_SetTargetRPM(MOTOR_RIGHT, signed_rpm);
        if (closed_loop_supervision_failed()) {
            return;
        }
    } else if (mode == TEST_LINE_FOLLOW) {
        if (!valid_line_follow_params()) {
            finish_run(TEST_STATUS_BAD_PARAMETER);
            return;
        }
        int sensor_state = line_follow_sensor_state(now);
        if (sensor_state == LINE_SENSOR_FAULTED) {
            return;
        }
        if (g_test.line.lap_phase ==
            LINE_LAP_PHASE_GENTLE_STOP) {
            if (update_line_lap(now)) {
                return;
            }
            if (g_test.elapsed_ms >= s_run_duration_ms) {
                finish_run(TEST_STATUS_TIMEOUT);
            }
            return;
        }
        if (g_test.line.lap_phase == LINE_LAP_PHASE_SETTLE) {
            set_line_zero_targets();
            if (sensor_state == LINE_SENSOR_VALID &&
                update_line_lap(now)) {
                return;
            }
            if (g_test.elapsed_ms >= s_run_duration_ms) {
                finish_run(TEST_STATUS_TIMEOUT);
            }
            return;
        }
        if (g_test.line.lap_phase ==
            LINE_LAP_PHASE_REVERSE_SEARCH) {
            if (sensor_state == LINE_SENSOR_VALID) {
                if (update_line_lap(now)) {
                    return;
                }
                set_line_reverse_targets();
                if (closed_loop_supervision_failed()) {
                    return;
                }
            } else {
                /*
                 * Forward directional recovery has the wrong geometry while
                 * the front sensor is trailing. Remove the reverse request
                 * and wait only inside the existing bounded loss window.
                 */
                set_line_zero_targets();
            }
            if (g_test.elapsed_ms >= s_run_duration_ms) {
                finish_run(TEST_STATUS_TIMEOUT);
            }
            return;
        }
        if (sensor_state == LINE_SENSOR_HOLDING) {
            if (closed_loop_supervision_failed()) {
                return;
            }
        } else if (sensor_state == LINE_SENSOR_RECOVERING) {
            set_line_recovery_targets();
            if (closed_loop_supervision_failed()) {
                return;
            }
        } else if (sensor_state == LINE_SENSOR_VALID) {
            if (update_line_lap(now)) {
                return;
            }
            if (g_test.line.lap_phase ==
                LINE_LAP_PHASE_GENTLE_STOP) {
                return;
            }
            if (g_test.line.lap_phase == LINE_LAP_PHASE_SETTLE) {
                set_line_zero_targets();
                return;
            }
            float base_rpm =
                g_test.line.lap_phase ==
                    LINE_LAP_PHASE_FORWARD_ALIGN ?
                        LINE_FINISH_FORWARD_ALIGN_RPM :
                        competition_base_rpm();
            if (s_supervisor_question_id == 3U &&
                g_test.line.lap_distance_mm >=
                    LINE_FOLLOW_Q3_APPROACH_START_MM) {
                base_rpm = LINE_FOLLOW_Q3_APPROACH_RPM;
            }
            if (s_supervisor_question_id == 3U &&
                g_test.line.lap_distance_mm >=
                    LINE_FOLLOW_Q3_FINAL_START_MM) {
                base_rpm = LINE_FINISH_APPROACH_RPM;
            }
            if (s_line_lap_active != 0U &&
                g_test.elapsed_ms < LINE_FOLLOW_START_DURATION_MS &&
                base_rpm > LINE_FOLLOW_START_BASE_RPM) {
                base_rpm = LINE_FOLLOW_START_BASE_RPM;
            } else if (s_line_lap_active != 0U &&
                g_test.line.lap_distance_mm <
                    LINE_FOLLOW_LAUNCH_DISTANCE_MM &&
                base_rpm > LINE_FOLLOW_LAUNCH_BASE_RPM) {
                base_rpm = LINE_FOLLOW_LAUNCH_BASE_RPM;
            }
            if (s_line_lap_active != 0U &&
                g_test.line.lap_distance_mm <
                    LINE_FOLLOW_START_STRAIGHT_MM) {
                set_line_straight_targets(base_rpm);
            } else {
                set_line_follow_targets(base_rpm);
            }
            if (closed_loop_supervision_failed()) {
                return;
            }
        }
    }

    if (g_test.elapsed_ms >= s_run_duration_ms) {
        if (is_closed_loop_mode(mode) &&
            (s_encoder_verified_mask & 3U) != 3U) {
            g_test.encoder_fault_mask =
                3U & ~s_encoder_verified_mask;
            finish_run(TEST_STATUS_ENCODER_FAULT);
        } else if (mode == TEST_LINE_FOLLOW &&
            s_line_lap_active != 0U) {
            finish_run(TEST_STATUS_TIMEOUT);
        } else {
            finish_run(TEST_STATUS_COMPLETE);
        }
    }
}
#endif

void TestMode_Init(void)
{
    Motor_Disarm();
    g_test.mode = TEST_IDLE;
    g_test.arm = 0;
    g_test.status = TEST_STATUS_IDLE;
    g_test.build_id = FW_BUILD_ID;
    g_test.elapsed_ms = 0U;
    g_test.button_mask = 0U;
    g_test.last_button_id = 0U;
    g_test.emergency_stop_latched = 0U;
    g_test.button_event_drops = 0U;
    g_test.start_delay_ms = 0U;
    g_test.motor_armed = 0U;
    g_test.output_fault_mask = 0U;
    g_test.encoder_fault_mask = 0U;
    g_test.encoder_verified_mask = 0U;
    g_test.left_invalid_transitions = 0U;
    g_test.left.encoder_count = 0;
    g_test.right.encoder_count = 0;
    g_test.left.rpm = 0.0f;
    g_test.right.rpm = 0.0f;
    g_test.left.error_rpm = 0.0f;
    g_test.right.error_rpm = 0.0f;
    g_test.left.pwm = 0;
    g_test.right.pwm = 0;
    g_test.left.integral = 0.0f;
    g_test.right.integral = 0.0f;
    g_test.left.pid_output = 0.0f;
    g_test.right.pid_output = 0.0f;
    g_test.line.lap_phase = LINE_LAP_PHASE_IDLE;
    g_test.line.turn_count = 0U;
    g_test.line.lap_yaw_deg = 0.0f;
    g_test.line.lap_distance_mm = 0.0f;
    g_test.line.finish_distance_mm = 0.0f;
    g_test.line.marker_recent_state = 0U;
    g_test.line.marker_recent_count = 0U;
    g_test.timer.running = 0U;
    g_test.timer.question_id = 0U;
    g_test.timer.power_enabled = 0U;
    g_test.timer.elapsed_ms = 0U;
    g_test.timer.elapsed_seconds = 0U;
    g_test.timer.oled_connected = 0U;
    g_test.timer.oled_address = 0U;
    g_test.timer.oled_error_count = 0U;
    g_test.timer.oled_requested_seconds = 0U;
    g_test.timer.oled_displayed_seconds = 0U;
    g_test.timer.oled_render_count = 0U;
#if TEST_MODE
    s_last_mode = -1;
    s_start_ms = Platform_Millis();
    s_run_duration_ms = TEST_DEFAULT_DURATION_MS;
    s_expected_left_direction = 0;
    s_expected_right_direction = 0;
    s_run_active = 0U;
    s_line_lap_active = 0U;
    s_line_common_motion_phase = TEST_COMMON_MOTION_STEADY;
    s_supervisor_question_id = 0U;
    s_button_raw_mask = read_button_mask();
    s_button_stable_mask = s_button_raw_mask;
    s_button_can_trigger_mask = (uint8_t) ~s_button_stable_mask;
    s_button_panel_initialized = 1U;
    s_emergency_stop_latched =
        (s_button_stable_mask & BUTTON8_MASK) != 0U ? 1U : 0U;
    s_power_enabled = 0U;
    s_button_event_head = 0U;
    s_button_event_tail = 0U;
    s_start_pending = 0U;
    s_start_source = START_SOURCE_NONE;
    s_start_mode = TEST_IDLE;
    s_start_duration_ms = 0U;
    uint32_t button_init_ms = Platform_Millis();
    for (uint8_t index = 0U; index < BUTTON_COUNT; index++) {
        s_button_raw_change_ms[index] = button_init_ms;
    }
    s_start_request_ms = button_init_ms;
    s_bluetooth_keepalive_ms = button_init_ms;
    s_start_sensor_check_ms = button_init_ms;
    s_start_sensor_sequence = 0U;
    s_start_sensor_check_active = 0U;
    s_start_sensor_valid_samples = 0U;
    g_test.button_mask = s_button_stable_mask;
    g_test.emergency_stop_latched = s_emergency_stop_latched;
    set_power_enabled(0U);
    if (s_emergency_stop_latched != 0U) {
        g_test.status = TEST_STATUS_EMERGENCY_STOP;
    }
    reset_encoder_watchdog();
#endif
    update_telemetry();
}

void TestMode_Stop(void)
{
#if TEST_MODE
    set_power_enabled(0U);
    cancel_pending_start();
    finish_run(s_emergency_stop_latched != 0U ?
        TEST_STATUS_EMERGENCY_STOP : TEST_STATUS_IDLE);
#else
    Motor_Disarm();
    g_test.mode = TEST_IDLE;
    g_test.arm = 0;
    g_test.status = TEST_STATUS_IDLE;
    update_telemetry();
#endif
}

int TestMode_TakeButtonEvent(
    uint8_t *button_id, uint8_t *button_action)
{
#if TEST_MODE
    if (button_id == 0 || button_action == 0 ||
        s_button_event_tail == s_button_event_head) {
        return 0;
    }
    *button_id = s_button_events[s_button_event_tail].id;
    *button_action = s_button_events[s_button_event_tail].action;
    s_button_event_tail = (uint8_t) ((s_button_event_tail + 1U) %
        BUTTON_EVENT_QUEUE_SIZE);
    return 1;
#else
    (void) button_id;
    (void) button_action;
    return 0;
#endif
}

int TestMode_IsEmergencyStopLatched(void)
{
#if TEST_MODE
    return s_emergency_stop_latched != 0U;
#else
    return 0;
#endif
}

int TestMode_IsButtonPanelOk(void)
{
#if TEST_MODE
    return s_button_panel_initialized != 0U;
#else
    return 0;
#endif
}

int TestMode_IsPowerEnabled(void)
{
#if TEST_MODE
    return s_power_enabled != 0U;
#else
    return 0;
#endif
}

uint8_t TestMode_GetCommonMotionPhase(void)
{
#if TEST_MODE
    return s_line_common_motion_phase;
#else
    return TEST_COMMON_MOTION_STEADY;
#endif
}

int TestMode_SupervisorPrepareQuestion(uint8_t question_id)
{
#if TEST_MODE
    const LineSensorData *line = LineSensor_GetData();
    if (question_id < 1U || question_id > 5U ||
        s_emergency_stop_latched != 0U ||
        s_run_active != 0U ||
        s_start_pending != 0U || g_test.mode != TEST_IDLE ||
        Motor_IsArmed()) {
        return 0;
    }
    if (question_id == 2U) {
        set_power_enabled(0U);
    } else if (s_power_enabled == 0U) {
        return 0;
    }
    if (question_id != 2U &&
        (line->connected == 0U ||
         line->line_lost != 0U ||
         !valid_line_follow_params() ||
         !valid_pid_gains(&g_test.left) ||
         !valid_pid_gains(&g_test.right))) {
        return 0;
    }
    s_supervisor_question_id = question_id;
    return 1;
#else
    (void) question_id;
    return 0;
#endif
}

int TestMode_SupervisorStartQuestion(
    uint8_t question_id, uint32_t timeout_ms)
{
#if TEST_MODE
    if (question_id < 1U || question_id > 5U ||
        timeout_ms < TEST_ENCODER_MIN_RUN_DURATION_MS ||
        timeout_ms > LINE_FOLLOW_MAX_DURATION_MS ||
        !TestMode_SupervisorPrepareQuestion(question_id)) {
        return 0;
    }

    if (question_id == 2U) {
        Motor_Disarm();
        clear_run_telemetry();
        s_start_pending = 0U;
        s_start_source = START_SOURCE_MC02;
        s_start_mode = TEST_IDLE;
        s_start_duration_ms = timeout_ms;
        s_run_duration_ms = timeout_ms;
        s_start_ms = Platform_Millis();
        s_run_active = 1U;
        g_test.mode = TEST_IDLE;
        g_test.arm = 0;
        g_test.status = TEST_STATUS_RUNNING;
        g_test.motor_armed = 0U;
        return 1;
    }

    if (!begin_start_delay(
            START_SOURCE_MC02, TEST_LINE_FOLLOW, timeout_ms)) {
        return 0;
    }

    /*
     * MC02 already owns the competition start sequence. Skip the former
     * local three-second delay, but retain the post-command fresh-sensor
     * gate before Motor_Arm().
     */
    s_start_request_ms = Platform_Millis() - START_DELAY_MS;
    g_test.start_delay_ms = 0U;
    return 1;
#else
    (void) question_id;
    (void) timeout_ms;
    return 0;
#endif
}

void TestMode_SupervisorStop(void)
{
    TestMode_Stop();
#if TEST_MODE
    s_supervisor_question_id = 0U;
#endif
}

int TestMode_IsRunActive(void)
{
#if TEST_MODE
    return s_run_active != 0U || s_start_pending != 0U;
#else
    return 0;
#endif
}

int TestMode_RequestBluetoothStart(void)
{
#if TEST_MODE
    return begin_start_delay(
        START_SOURCE_BLUETOOTH, TEST_CLOSED_LOOP_RPM,
        RUN_DURATION_MS);
#else
    return 0;
#endif
}

int TestMode_RequestBluetoothLineFollow(void)
{
#if TEST_MODE
    return begin_start_delay(
        START_SOURCE_BLUETOOTH, TEST_LINE_FOLLOW,
        LINE_FOLLOW_TEST_DURATION_MS);
#else
    return 0;
#endif
}

int TestMode_RequestBluetoothLineLap(void)
{
#if TEST_MODE
    return begin_start_delay(
        START_SOURCE_BLUETOOTH, TEST_LINE_FOLLOW,
        LINE_FOLLOW_LAP_DURATION_MS);
#else
    return 0;
#endif
}

void TestMode_BluetoothKeepalive(void)
{
#if TEST_MODE
    if (s_start_source == START_SOURCE_BLUETOOTH &&
        (s_start_pending != 0U || s_run_active != 0U)) {
        s_bluetooth_keepalive_ms = Platform_Millis();
    }
#endif
}

void TestMode_Tick(void)
{
#if TEST_MODE
    update_user_buttons();
    g_test.emergency_stop_latched = s_emergency_stop_latched;
    int32_t mode = g_test.mode;
    if (s_run_active != 0U &&
        s_supervisor_question_id == 2U) {
        /*
         * Do not permit a debugger write or stale mode value to grant motor
         * authority during the stationary question.
         */
        Motor_Disarm();
        g_test.mode = TEST_IDLE;
        g_test.arm = 0;
        mode = TEST_IDLE;
        s_last_mode = TEST_IDLE;
    }
    if (mode != s_last_mode) {
        enter_mode(mode);
        s_last_mode = g_test.mode;
    }
    if (s_run_active != 0U) {
        update_running_mode(mode);
    }
    update_telemetry();
#else
    Motor_Disarm();
    g_test.mode = TEST_IDLE;
    g_test.arm = 0;
    g_test.status = TEST_STATUS_IDLE;
    update_telemetry();
#endif
}
