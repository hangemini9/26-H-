#include "motor.h"

#include <float.h>

#include "platform.h"
#include "ti_msp_dl_config.h"

static Motor_TypeDef g_motor[MOTOR_NUM];

static volatile uint32_t s_left_encoder_raw;
static volatile uint8_t s_left_encoder_state;
static volatile uint32_t s_left_encoder_invalid_transitions;
static volatile uint8_t s_motor_armed;
static uint8_t s_pwm_timer_pin_mask;
static uint32_t s_pwm_period_counts;
static int8_t s_raw_direction[MOTOR_NUM];
static int8_t s_pending_direction[MOTOR_NUM];
static uint8_t s_direction_phase[MOTOR_NUM];
static uint32_t s_direction_phase_start_ms[MOTOR_NUM];

enum {
    DIRECTION_PHASE_IDLE = 0,
    DIRECTION_PHASE_COAST,
    DIRECTION_PHASE_SETUP
};

#define PWM_TIMER_PIN_RIGHT (1U << MOTOR_RIGHT)
#define PWM_TIMER_PIN_LEFT  (1U << MOTOR_LEFT)
#define PWM_TIMER_PIN_ALL   (PWM_TIMER_PIN_RIGHT | PWM_TIMER_PIN_LEFT)

static const uint32_t s_counts_per_wheel_rev[MOTOR_NUM] = {
    [MOTOR_RIGHT] = ENCODER_RIGHT_COUNTS_PER_WHEEL_REV,
    [MOTOR_LEFT] = ENCODER_LEFT_COUNTS_PER_WHEEL_REV,
};

/* Index: previous AB in bits 3:2, current AB in bits 1:0. */
static const int8_t s_quadrature_delta[16] = {
     0, -1,  1,  0,
     1,  0,  0, -1,
    -1,  0,  0,  1,
     0,  1, -1,  0,
};

static int float_is_finite(float value)
{
    return value == value && value <= FLT_MAX && value >= -FLT_MAX;
}

static float abs_float(float value)
{
    return value < 0.0f ? -value : value;
}

static void pid_reset(PID_TypeDef *pid)
{
    pid->integral = 0.0f;
    pid->prev_error = 0.0f;
    pid->output = 0.0f;
}

static void pid_init(PID_TypeDef *pid)
{
    /*
     * Provisional values equivalent to the old 10 ms controller, expressed
     * with integral units in seconds. Retune on the loaded chassis.
     */
    pid->kp = MOTOR_PID_DEFAULT_KP;
    pid->ki = MOTOR_PID_DEFAULT_KI;
    pid->kd = MOTOR_PID_DEFAULT_KD;
    pid->integral_max = 6.0f;
    pid->out_max = MOTOR_CLOSED_LOOP_PWM_LIMIT;
    pid->out_min = -MOTOR_CLOSED_LOOP_PWM_LIMIT;
    pid_reset(pid);
}

static void force_pwm_pin_low(Motor_ID id);
static void connect_pwm_pin_to_timer(Motor_ID id);

static uint8_t read_left_encoder_state(void)
{
    uint32_t pins = DL_GPIO_readPins(GPIO_ENCODER_LEFT_PORT,
        GPIO_ENCODER_LEFT_ENCODER_LEFT_A_PIN |
            GPIO_ENCODER_LEFT_ENCODER_LEFT_B_PIN);
    uint8_t state = 0U;

    if ((pins & GPIO_ENCODER_LEFT_ENCODER_LEFT_A_PIN) != 0U) {
        state |= 2U;
    }
    if ((pins & GPIO_ENCODER_LEFT_ENCODER_LEFT_B_PIN) != 0U) {
        state |= 1U;
    }
    return state;
}

static void set_pwm_percent(Motor_ID id, uint16_t duty_percent)
{
    if (duty_percent > (uint16_t) MOTOR_PWM_MAX) {
        duty_percent = (uint16_t) MOTOR_PWM_MAX;
    }

    if (s_pwm_period_counts == 0U) {
        return;
    }

    /*
     * In this down-count edge-aligned configuration, LOAD drives CCP high
     * and a compare-down event drives it low. A compare value equal to the
     * period is outside the counter range, so it produces constant high,
     * not zero duty. A true 0% command must therefore use GPIO-low.
     */
    if (duty_percent == 0U) {
        force_pwm_pin_low(id);
        return;
    }

    uint32_t compare = s_pwm_period_counts -
        ((s_pwm_period_counts * (uint32_t) duty_percent) / 100U);
    DL_TIMER_CC_INDEX channel = (id == MOTOR_RIGHT) ?
        GPIO_MOTOR_PWM_C0_IDX : GPIO_MOTOR_PWM_C1_IDX;
    DL_TimerG_setCaptureCompareValue(MOTOR_PWM_INST, compare, channel);
    connect_pwm_pin_to_timer(id);
}

static void set_direction(Motor_ID id, int direction)
{
    uint32_t pin = (id == MOTOR_RIGHT) ?
        GPIO_MOTOR_RIGHT_DIR_PIN : GPIO_MOTOR_LEFT_DIR_PIN;

    if (direction > 0) {
        DL_GPIO_setPins(GPIO_MOTOR_PORT, pin);
    } else {
        DL_GPIO_clearPins(GPIO_MOTOR_PORT, pin);
    }
}

static void force_outputs_off(void)
{
    if (s_pwm_period_counts == 0U) {
        return;
    }
    DL_TimerG_setCaptureCompareValue(MOTOR_PWM_INST,
        s_pwm_period_counts, GPIO_MOTOR_PWM_C0_IDX);
    DL_TimerG_setCaptureCompareValue(MOTOR_PWM_INST,
        s_pwm_period_counts, GPIO_MOTOR_PWM_C1_IDX);
}

static void force_pwm_pin_low(Motor_ID id)
{
    uint8_t mask = (id == MOTOR_RIGHT) ?
        PWM_TIMER_PIN_RIGHT : PWM_TIMER_PIN_LEFT;
    GPIO_Regs *port = (id == MOTOR_RIGHT) ?
        GPIO_MOTOR_PWM_C0_PORT : GPIO_MOTOR_PWM_C1_PORT;
    uint32_t pin = (id == MOTOR_RIGHT) ?
        GPIO_MOTOR_PWM_C0_PIN : GPIO_MOTOR_PWM_C1_PIN;
    uint32_t iomux = (id == MOTOR_RIGHT) ?
        GPIO_MOTOR_PWM_C0_IOMUX : GPIO_MOTOR_PWM_C1_IOMUX;
    DL_TIMER_CC_INDEX channel = (id == MOTOR_RIGHT) ?
        GPIO_MOTOR_PWM_C0_IDX : GPIO_MOTOR_PWM_C1_IDX;

    DL_TimerG_setCaptureCompareValue(
        MOTOR_PWM_INST, s_pwm_period_counts, channel);
    DL_GPIO_clearPins(port, pin);
    if ((s_pwm_timer_pin_mask & mask) != 0U) {
        DL_GPIO_initDigitalOutput(iomux);
        DL_GPIO_enableOutput(port, pin);
        s_pwm_timer_pin_mask &= (uint8_t) ~mask;
    }
    DL_GPIO_clearPins(port, pin);
}

static void connect_pwm_pin_to_timer(Motor_ID id)
{
    uint8_t mask = (id == MOTOR_RIGHT) ?
        PWM_TIMER_PIN_RIGHT : PWM_TIMER_PIN_LEFT;

    if ((s_pwm_timer_pin_mask & mask) != 0U) {
        return;
    }

    if (id == MOTOR_RIGHT) {
        DL_GPIO_initPeripheralOutputFunction(
            GPIO_MOTOR_PWM_C0_IOMUX, GPIO_MOTOR_PWM_C0_IOMUX_FUNC);
        DL_GPIO_enableOutput(
            GPIO_MOTOR_PWM_C0_PORT, GPIO_MOTOR_PWM_C0_PIN);
    } else {
        DL_GPIO_initPeripheralOutputFunction(
            GPIO_MOTOR_PWM_C1_IOMUX, GPIO_MOTOR_PWM_C1_IOMUX_FUNC);
        DL_GPIO_enableOutput(
            GPIO_MOTOR_PWM_C1_PORT, GPIO_MOTOR_PWM_C1_PIN);
    }
    s_pwm_timer_pin_mask |= mask;
}

/*
 * Disarming must produce a physical low level even if the timer is halted,
 * misconfigured, or the external PWM/DIR wires have accidentally been
 * crossed. Disconnect both CCP functions from the pins and drive all four
 * motor-control signals low.
 */
static void force_control_pins_low(void)
{
    force_outputs_off();
    force_pwm_pin_low(MOTOR_RIGHT);
    force_pwm_pin_low(MOTOR_LEFT);
    DL_GPIO_clearPins(GPIO_MOTOR_PORT,
        GPIO_MOTOR_RIGHT_DIR_PIN | GPIO_MOTOR_LEFT_DIR_PIN);
}

static void cancel_direction_transition(Motor_ID id)
{
    s_pending_direction[id] = 0;
    s_direction_phase[id] = DIRECTION_PHASE_IDLE;
    s_direction_phase_start_ms[id] = 0U;
    set_direction(id, s_raw_direction[id]);
}

static int direction_is_ready(Motor_ID id, int8_t requested_direction)
{
    uint32_t now = Platform_Millis();

    if (requested_direction == 0) {
        s_pending_direction[id] = 0;
        s_direction_phase[id] = DIRECTION_PHASE_IDLE;
        s_direction_phase_start_ms[id] = 0U;
        s_raw_direction[id] = 0;
        set_direction(id, 0);
        return 0;
    }

    if (s_direction_phase[id] == DIRECTION_PHASE_SETUP) {
        if (requested_direction != s_pending_direction[id]) {
            /*
             * PWM has remained off throughout setup, so changing PH/DIR again
             * only requires restarting the setup interval.
             */
            set_direction(id, requested_direction);
            s_pending_direction[id] = requested_direction;
            s_direction_phase_start_ms[id] = now;
            return 0;
        }
        if ((uint32_t) (now - s_direction_phase_start_ms[id]) <
            MOTOR_DIRECTION_SETUP_MS) {
            return 0;
        }
        s_raw_direction[id] = requested_direction;
        s_pending_direction[id] = 0;
        s_direction_phase[id] = DIRECTION_PHASE_IDLE;
        return 1;
    }

    if (s_direction_phase[id] == DIRECTION_PHASE_COAST) {
        if (requested_direction == s_raw_direction[id]) {
            cancel_direction_transition(id);
            return 1;
        }
        if (requested_direction != s_pending_direction[id]) {
            s_pending_direction[id] = requested_direction;
            s_direction_phase_start_ms[id] = now;
        }

        uint32_t coast_ms =
            (uint32_t) (now - s_direction_phase_start_ms[id]);
        /*
         * Always enforce the full coast interval. A disconnected encoder
         * reads as zero RPM, so feedback alone must never shorten this wait.
         */
        if (coast_ms < MOTOR_REVERSE_COAST_MS) {
            return 0;
        }
        if (!float_is_finite(g_motor[id].current_rpm) ||
            abs_float(g_motor[id].current_rpm) > MOTOR_REVERSE_SAFE_RPM) {
            return 0;
        }

        set_direction(id, requested_direction);
        s_direction_phase[id] = DIRECTION_PHASE_SETUP;
        s_direction_phase_start_ms[id] = now;
        return 0;
    }

    if (s_raw_direction[id] == 0) {
        set_direction(id, requested_direction);
        s_pending_direction[id] = requested_direction;
        s_direction_phase[id] = DIRECTION_PHASE_SETUP;
        s_direction_phase_start_ms[id] = now;
        return 0;
    }

    if (requested_direction != s_raw_direction[id]) {
        s_pending_direction[id] = requested_direction;
        s_direction_phase[id] = DIRECTION_PHASE_COAST;
        s_direction_phase_start_ms[id] = now;
        return 0;
    }

    return 1;
}

static void apply_pwm(Motor_ID id, int16_t pwm)
{
    if (pwm > MOTOR_PWM_MAX) {
        pwm = MOTOR_PWM_MAX;
    } else if (pwm < -MOTOR_PWM_MAX) {
        pwm = -MOTOR_PWM_MAX;
    }

    if (s_motor_armed == 0U) {
        pwm = 0;
    }

    int16_t raw_pwm = pwm;
    if ((id == MOTOR_LEFT && MOTOR_LEFT_OUTPUT_REVERSED != 0) ||
        (id == MOTOR_RIGHT && MOTOR_RIGHT_OUTPUT_REVERSED != 0)) {
        raw_pwm = (int16_t) -raw_pwm;
    }

    int8_t new_direction = 0;
    if (raw_pwm > 0) {
        new_direction = 1;
    } else if (raw_pwm < 0) {
        new_direction = -1;
    }

    uint32_t primask = Platform_EnterCritical();
    if (!direction_is_ready(id, new_direction)) {
        set_pwm_percent(id, 0U);
        g_motor[id].pwm = 0;
    } else if (new_direction > 0) {
        set_pwm_percent(id, (uint16_t) raw_pwm);
        g_motor[id].pwm = pwm;
    } else if (new_direction < 0) {
        set_pwm_percent(id, (uint16_t) -raw_pwm);
        g_motor[id].pwm = pwm;
    } else {
        set_pwm_percent(id, 0U);
        g_motor[id].pwm = 0;
    }
    Platform_ExitCritical(primask);
}

static float pid_compute(PID_TypeDef *pid, float setpoint, float measure)
{
    if (!float_is_finite(setpoint) || !float_is_finite(measure) ||
        !float_is_finite(pid->kp) || !float_is_finite(pid->ki) ||
        !float_is_finite(pid->kd) || !float_is_finite(pid->integral_max) ||
        !float_is_finite(pid->out_max) || !float_is_finite(pid->out_min) ||
        pid->kp < 0.0f || pid->ki < 0.0f || pid->kd < 0.0f ||
        pid->integral_max < 0.0f) {
        pid_reset(pid);
        return 0.0f;
    }

    float effective_out_max = pid->out_max;
    float effective_out_min = pid->out_min;
    if (effective_out_max > MOTOR_CLOSED_LOOP_PWM_LIMIT) {
        effective_out_max = MOTOR_CLOSED_LOOP_PWM_LIMIT;
    }
    if (effective_out_min < -MOTOR_CLOSED_LOOP_PWM_LIMIT) {
        effective_out_min = -MOTOR_CLOSED_LOOP_PWM_LIMIT;
    }
    if (effective_out_max <= 0.0f || effective_out_min >= 0.0f ||
        effective_out_min >= effective_out_max) {
        pid_reset(pid);
        return 0.0f;
    }

    float error = setpoint - measure;
    const float dt_s = (float) MOTOR_CTRL_PERIOD_MS * 0.001f;
    float next_integral = pid->integral + error * dt_s;

    if (next_integral > pid->integral_max) {
        next_integral = pid->integral_max;
    } else if (next_integral < -pid->integral_max) {
        next_integral = -pid->integral_max;
    }

    float derivative = (error - pid->prev_error) / dt_s;
    pid->prev_error = error;
    float output = pid->kp * error + pid->ki * next_integral +
        pid->kd * derivative;

    if (!float_is_finite(output)) {
        pid_reset(pid);
        return 0.0f;
    }

    if (output > effective_out_max) {
        output = effective_out_max;
        if (error <= 0.0f) {
            pid->integral = next_integral;
        }
    } else if (output < effective_out_min) {
        output = effective_out_min;
        if (error >= 0.0f) {
            pid->integral = next_integral;
        }
    } else {
        pid->integral = next_integral;
    }

    pid->output = output;
    return output;
}

static uint32_t read_raw_count(Motor_ID id)
{
    if (id == MOTOR_RIGHT) {
        return (uint32_t) (uint16_t)
            DL_TimerG_getTimerCount(QEI_RIGHT_INST);
    }

    uint32_t primask = Platform_EnterCritical();
    uint32_t count = s_left_encoder_raw;
    Platform_ExitCritical(primask);
    return count;
}

static int32_t modular_delta_u32(uint32_t current, uint32_t previous)
{
    uint32_t difference = current - previous;
    if (difference <= (uint32_t) INT32_MAX) {
        return (int32_t) difference;
    }

    uint32_t magnitude = 0U - difference;
    if (magnitude > (uint32_t) INT32_MAX) {
        return INT32_MIN;
    }
    return -(int32_t) magnitude;
}

static int32_t modular_delta_u16(uint16_t current, uint16_t previous)
{
    uint16_t difference = (uint16_t) (current - previous);
    if (difference <= (uint16_t) INT16_MAX) {
        return (int32_t) difference;
    }
    return -(int32_t) (uint16_t) (0U - difference);
}

static void encoder_update(Motor_ID id)
{
    Motor_TypeDef *motor = &g_motor[id];
    uint32_t raw = read_raw_count(id);
    int32_t delta;

    if (id == MOTOR_RIGHT) {
        delta = modular_delta_u16(
            (uint16_t) raw, (uint16_t) motor->encoder.last_count);
    } else {
        delta = modular_delta_u32(raw, motor->encoder.last_count);
    }

    if ((id == MOTOR_LEFT && ENCODER_LEFT_REVERSED != 0) ||
        (id == MOTOR_RIGHT && ENCODER_RIGHT_REVERSED != 0)) {
        delta = -delta;
    }

    motor->encoder.current_count = raw;
    motor->encoder.last_count = raw;
    motor->encoder.delta_count = delta;
    if (delta > 0 &&
        motor->encoder.total_count > INT64_MAX - (int64_t) delta) {
        motor->encoder.total_count = INT64_MAX;
    } else if (delta < 0 &&
               motor->encoder.total_count < INT64_MIN - (int64_t) delta) {
        motor->encoder.total_count = INT64_MIN;
    } else {
        motor->encoder.total_count += (int64_t) delta;
    }

    float raw_rpm = ((float) delta * 60000.0f) /
        ((float) s_counts_per_wheel_rev[id] *
            (float) MOTOR_CTRL_PERIOD_MS);
    float rpm = motor->encoder.speed_rpm +
        MOTOR_RPM_FILTER_ALPHA *
            (raw_rpm - motor->encoder.speed_rpm);
    motor->encoder.raw_speed_rpm = raw_rpm;
    motor->encoder.speed_rpm = rpm;
    motor->encoder.speed_radps =
        rpm * (2.0f * CHASSIS_PI / 60.0f);
    motor->current_rpm = rpm;
}

void Motor_Init(void)
{
    s_motor_armed = 0U;
    s_pwm_timer_pin_mask = PWM_TIMER_PIN_ALL;
    /*
     * DriverLib stores edge-aligned PWM LOAD as period - 1. Compare equal to
     * the original period is SysConfig's guaranteed zero-duty value.
     */
    s_pwm_period_counts =
        DL_TimerG_getLoadValue(MOTOR_PWM_INST) + 1U;
    s_raw_direction[MOTOR_LEFT] = 0;
    s_raw_direction[MOTOR_RIGHT] = 0;
    s_pending_direction[MOTOR_LEFT] = 0;
    s_pending_direction[MOTOR_RIGHT] = 0;
    s_direction_phase[MOTOR_LEFT] = DIRECTION_PHASE_IDLE;
    s_direction_phase[MOTOR_RIGHT] = DIRECTION_PHASE_IDLE;
    s_direction_phase_start_ms[MOTOR_LEFT] = 0U;
    s_direction_phase_start_ms[MOTOR_RIGHT] = 0U;
    DL_GPIO_clearPins(GPIO_MOTOR_PORT,
        GPIO_MOTOR_RIGHT_DIR_PIN | GPIO_MOTOR_LEFT_DIR_PIN);
    force_control_pins_low();

    for (int i = 0; i < (int) MOTOR_NUM; i++) {
        g_motor[i].encoder.last_count = 0;
        g_motor[i].encoder.current_count = 0;
        g_motor[i].encoder.delta_count = 0;
        g_motor[i].encoder.total_count = 0;
        g_motor[i].encoder.raw_speed_rpm = 0.0f;
        g_motor[i].encoder.speed_rpm = 0.0f;
        g_motor[i].encoder.speed_radps = 0.0f;
        g_motor[i].target_rpm = 0.0f;
        g_motor[i].current_rpm = 0.0f;
        g_motor[i].requested_pwm = 0;
        g_motor[i].pwm = 0;
        g_motor[i].closed_loop = 0U;
        pid_init(&g_motor[i].pid);
    }

    s_left_encoder_raw = 0;
    s_left_encoder_state = read_left_encoder_state();
    s_left_encoder_invalid_transitions = 0U;
    DL_TimerG_setTimerCount(QEI_RIGHT_INST, 0U);

    DL_GPIO_clearInterruptStatus(GPIO_ENCODER_LEFT_PORT,
        GPIO_ENCODER_LEFT_ENCODER_LEFT_A_PIN |
            GPIO_ENCODER_LEFT_ENCODER_LEFT_B_PIN);
    DL_TimerG_startCounter(QEI_RIGHT_INST);
    DL_TimerG_startCounter(MOTOR_PWM_INST);
    force_control_pins_low();
}

void Motor_ControlTick(void)
{
    for (int i = 0; i < (int) MOTOR_NUM; i++) {
        encoder_update((Motor_ID) i);
    }

    if (s_motor_armed == 0U) {
        force_control_pins_low();
        g_motor[MOTOR_LEFT].requested_pwm = 0;
        g_motor[MOTOR_RIGHT].requested_pwm = 0;
        g_motor[MOTOR_LEFT].pwm = 0;
        g_motor[MOTOR_RIGHT].pwm = 0;
        return;
    }

    for (int i = 0; i < (int) MOTOR_NUM; i++) {
        Motor_ID id = (Motor_ID) i;
        int16_t requested_pwm = g_motor[i].requested_pwm;
        if (g_motor[i].closed_loop != 0U) {
            if (g_motor[i].target_rpm > -0.01f &&
                g_motor[i].target_rpm < 0.01f) {
                pid_reset(&g_motor[i].pid);
                requested_pwm = 0;
            } else {
                float output = pid_compute(&g_motor[i].pid,
                    g_motor[i].target_rpm, g_motor[i].current_rpm);
                /*
                 * Commissioning speed control never applies active torque in
                 * the direction opposite to its non-zero speed target.
                 */
                if ((g_motor[i].target_rpm > 0.0f && output < 0.0f) ||
                    (g_motor[i].target_rpm < 0.0f && output > 0.0f)) {
                    pid_reset(&g_motor[i].pid);
                    output = 0.0f;
                }
                requested_pwm = (int16_t) output;
            }
        }
        apply_pwm(id, requested_pwm);
    }
}

void Motor_Arm(void)
{
    uint32_t primask = Platform_EnterCritical();
    force_control_pins_low();
    if (s_pwm_period_counts != 0U) {
        DL_TimerG_stopCounter(MOTOR_PWM_INST);
        DL_TimerG_setTimerCount(MOTOR_PWM_INST, 0U);
        DL_TimerG_startCounter(MOTOR_PWM_INST);
        s_motor_armed = 1U;
    }
    Platform_ExitCritical(primask);
}

void Motor_Disarm(void)
{
    uint32_t primask = Platform_EnterCritical();
    s_motor_armed = 0U;
    force_control_pins_low();
    s_raw_direction[MOTOR_LEFT] = 0;
    s_raw_direction[MOTOR_RIGHT] = 0;
    s_pending_direction[MOTOR_LEFT] = 0;
    s_pending_direction[MOTOR_RIGHT] = 0;
    s_direction_phase[MOTOR_LEFT] = DIRECTION_PHASE_IDLE;
    s_direction_phase[MOTOR_RIGHT] = DIRECTION_PHASE_IDLE;
    s_direction_phase_start_ms[MOTOR_LEFT] = 0U;
    s_direction_phase_start_ms[MOTOR_RIGHT] = 0U;
    Platform_ExitCritical(primask);

    for (int i = 0; i < (int) MOTOR_NUM; i++) {
        g_motor[i].closed_loop = 0U;
        g_motor[i].target_rpm = 0.0f;
        g_motor[i].requested_pwm = 0;
        g_motor[i].pwm = 0;
        pid_reset(&g_motor[i].pid);
    }
}

int Motor_IsArmed(void)
{
    return s_motor_armed != 0U;
}

void Motor_SetPWM(Motor_ID id, int16_t pwm)
{
    if (id >= MOTOR_NUM) {
        return;
    }
    if (pwm > MOTOR_PWM_MAX) {
        pwm = MOTOR_PWM_MAX;
    } else if (pwm < -MOTOR_PWM_MAX) {
        pwm = -MOTOR_PWM_MAX;
    }
    g_motor[id].closed_loop = 0U;
    g_motor[id].target_rpm = 0.0f;
    g_motor[id].requested_pwm = pwm;
    pid_reset(&g_motor[id].pid);
    if (pwm == 0) {
        apply_pwm(id, 0);
    }
}

void Motor_SetTargetRPM(Motor_ID id, float rpm)
{
    if (id >= MOTOR_NUM) {
        return;
    }
    if (!float_is_finite(rpm)) {
        rpm = 0.0f;
    }
    if (rpm > MOTOR_MAX_RPM) {
        rpm = MOTOR_MAX_RPM;
    } else if (rpm < -MOTOR_MAX_RPM) {
        rpm = -MOTOR_MAX_RPM;
    }
    if (g_motor[id].closed_loop == 0U) {
        pid_reset(&g_motor[id].pid);
    }
    g_motor[id].closed_loop = 1U;
    g_motor[id].requested_pwm = 0;
    g_motor[id].target_rpm = rpm;
}

void Motor_Stop(void)
{
    for (int i = 0; i < (int) MOTOR_NUM; i++) {
        g_motor[i].closed_loop = 0U;
        g_motor[i].target_rpm = 0.0f;
        g_motor[i].requested_pwm = 0;
        pid_reset(&g_motor[i].pid);
        apply_pwm((Motor_ID) i, 0);
    }
}

void Motor_PID_SetParams(Motor_ID id, float kp, float ki, float kd)
{
    if (id >= MOTOR_NUM || !float_is_finite(kp) ||
        !float_is_finite(ki) || !float_is_finite(kd) ||
        kp < 0.0f || ki < 0.0f || kd < 0.0f) {
        return;
    }
    g_motor[id].pid.kp = kp;
    g_motor[id].pid.ki = ki;
    g_motor[id].pid.kd = kd;
    pid_reset(&g_motor[id].pid);
}

float Motor_GetSpeedRPM(Motor_ID id)
{
    return (id < MOTOR_NUM) ? g_motor[id].current_rpm : 0.0f;
}

float Motor_GetSpeedRadPS(Motor_ID id)
{
    return (id < MOTOR_NUM) ? g_motor[id].encoder.speed_radps : 0.0f;
}

int64_t Motor_GetTotalCount(Motor_ID id)
{
    return (id < MOTOR_NUM) ? g_motor[id].encoder.total_count : 0;
}

int16_t Motor_GetAppliedPWM(Motor_ID id)
{
    return (id < MOTOR_NUM) ? g_motor[id].pwm : 0;
}

float Motor_GetPIDIntegral(Motor_ID id)
{
    return (id < MOTOR_NUM) ? g_motor[id].pid.integral : 0.0f;
}

float Motor_GetPIDOutput(Motor_ID id)
{
    return (id < MOTOR_NUM) ? g_motor[id].pid.output : 0.0f;
}

uint32_t Motor_GetCountsPerWheelRev(Motor_ID id)
{
    return (id < MOTOR_NUM) ? s_counts_per_wheel_rev[id] : 0U;
}

static void reset_encoders(int reset_left, int reset_right)
{
    uint32_t primask = Platform_EnterCritical();

    if (reset_left) {
        s_left_encoder_raw = 0;
        s_left_encoder_state = read_left_encoder_state();
        s_left_encoder_invalid_transitions = 0U;
    }
    if (reset_right) {
        DL_TimerG_setTimerCount(QEI_RIGHT_INST, 0U);
    }

    for (int i = 0; i < (int) MOTOR_NUM; i++) {
        if ((i == (int) MOTOR_LEFT && !reset_left) ||
            (i == (int) MOTOR_RIGHT && !reset_right)) {
            continue;
        }
        g_motor[i].encoder.last_count = 0;
        g_motor[i].encoder.current_count = 0;
        g_motor[i].encoder.delta_count = 0;
        g_motor[i].encoder.total_count = 0;
        g_motor[i].encoder.raw_speed_rpm = 0.0f;
        g_motor[i].encoder.speed_rpm = 0.0f;
        g_motor[i].encoder.speed_radps = 0.0f;
        g_motor[i].current_rpm = 0.0f;
    }
    Platform_ExitCritical(primask);
}

void Motor_ResetEncoder(Motor_ID id)
{
    if (id == MOTOR_LEFT) {
        reset_encoders(1, 0);
    } else if (id == MOTOR_RIGHT) {
        reset_encoders(0, 1);
    }
}

void Motor_ResetAllEncoders(void)
{
    reset_encoders(1, 1);
}

void Motor_LeftEncoderGPIOIRQ(void)
{
    uint8_t current = read_left_encoder_state();
    uint8_t transition =
        (uint8_t) ((s_left_encoder_state << 2U) | current);
    int8_t delta = s_quadrature_delta[transition & 0x0FU];
    if (current != s_left_encoder_state && delta == 0) {
        s_left_encoder_invalid_transitions++;
    }
    s_left_encoder_raw += (uint32_t) (int32_t) delta;
    s_left_encoder_state = current;
}

uint32_t Motor_GetLeftEncoderInvalidTransitions(void)
{
    return s_left_encoder_invalid_transitions;
}
