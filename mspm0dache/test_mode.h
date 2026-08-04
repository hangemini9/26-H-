#ifndef TEST_MODE_H
#define TEST_MODE_H

#include <stdint.h>

#ifndef TEST_MODE
#define TEST_MODE 1
#endif

typedef enum {
    TEST_IDLE = 0,
    TEST_ENCODER_OBSERVE = 1,
    TEST_ENCODER_RESET = 2,
    TEST_RIGHT_OPEN_LOOP = 3,
    TEST_LEFT_OPEN_LOOP = 4,
    TEST_BOTH_OPEN_FORWARD = 5,
    TEST_BOTH_OPEN_REVERSE = 6,
    TEST_CLOSED_LOOP_RPM = 7,
    TEST_LINE_FOLLOW = 8
} TestCommand;

typedef enum {
    TEST_STATUS_EMERGENCY_STOP = -7,
    TEST_STATUS_LINE_SENSOR_FAULT = -6,
    TEST_STATUS_OUTPUT_FAULT = -5,
    TEST_STATUS_ENCODER_FAULT = -4,
    TEST_STATUS_BAD_PARAMETER = -3,
    TEST_STATUS_TIMEOUT = -2,
    TEST_STATUS_NOT_ARMED = -1,
    TEST_STATUS_IDLE = 0,
    TEST_STATUS_RUNNING = 1,
    TEST_STATUS_COMPLETE = 2,
    TEST_STATUS_START_DELAY = 3,
    TEST_STATUS_FINISH_DISTANCE_FALLBACK = 4
} TestStatus;

typedef enum {
    LINE_LAP_PHASE_IDLE = 0,
    LINE_LAP_PHASE_LEAVE_START = 1,
    LINE_LAP_PHASE_SEARCH_FINISH = 2,
    LINE_LAP_PHASE_FORWARD_ALIGN = 3,
    LINE_LAP_PHASE_SETTLE = 4,
    LINE_LAP_PHASE_REVERSE_SEARCH = 5,
    LINE_LAP_PHASE_MARKER_DETECTED = 6,
    LINE_LAP_PHASE_GENTLE_STOP = 7,
    LINE_LAP_PHASE_COMPLETE = 8
} LineLapPhase;

typedef enum {
    TEST_COMMON_MOTION_STEADY = 0,
    TEST_COMMON_MOTION_ACCELERATING = 1,
    TEST_COMMON_MOTION_DECELERATING = 2
} TestCommonMotionPhase;

typedef struct {
    volatile int64_t encoder_count;
    volatile float rpm;
    volatile float error_rpm;
    volatile int32_t pwm;
    volatile float kp;
    volatile float ki;
    volatile float kd;
    volatile float integral;
    volatile float pid_output;
} ChassisTestWheel;

typedef struct {
    volatile uint32_t connected;
    volatile uint32_t sample_sequence;
    volatile uint32_t error_count;
    volatile uint32_t state;
    volatile uint32_t active_count;
    volatile uint32_t line_lost;
    volatile int32_t position_x1000;
    volatile float base_rpm;
    volatile float steering_kp;
    volatile float steering_kd;
    volatile float correction_rpm;
    volatile float left_target_rpm;
    volatile float right_target_rpm;
    volatile uint32_t lap_phase;
    volatile uint32_t turn_count;
    volatile float lap_yaw_deg;
    volatile float lap_distance_mm;
    volatile float finish_distance_mm;
    volatile uint32_t marker_recent_state;
    volatile uint32_t marker_recent_count;
    volatile uint16_t analog[8];
    volatile uint16_t threshold[8];
} ChassisTestLineSensor;

typedef struct {
    volatile uint32_t online;
    volatile uint32_t state;
    volatile uint32_t fault;
    volatile uint32_t run_id;
    volatile uint32_t last_command_id;
    volatile uint32_t question_id;
    volatile uint32_t rx_frames;
    volatile uint32_t crc_errors;
    volatile uint32_t tx_frames;
} ChassisTestMc02Link;

typedef struct {
    volatile uint32_t running;
    volatile uint32_t question_id;
    volatile uint32_t power_enabled;
    volatile uint32_t elapsed_ms;
    volatile uint32_t elapsed_seconds;
    volatile uint32_t oled_connected;
    volatile uint32_t oled_address;
    volatile uint32_t oled_error_count;
    volatile uint32_t oled_requested_seconds;
    volatile uint32_t oled_displayed_seconds;
    volatile uint32_t oled_render_count;
} ChassisTestQuestionTimer;

/*
 * The only debugger/WATCH interface.
 *
 * Write duration_ms, target_rpm, left/right kp/ki/kd, and for mode 8
 * line.base_rpm/line.steering_kp while stopped. Then write arm = 1 and mode
 * last. All other fields are telemetry.
 */
typedef struct {
    volatile int32_t mode;
    volatile int32_t arm;
    volatile int32_t status;
    volatile uint32_t build_id;
    volatile uint32_t duration_ms;
    volatile float target_rpm;
    volatile uint32_t elapsed_ms;
    volatile uint32_t button_mask;
    volatile uint32_t last_button_id;
    volatile uint32_t emergency_stop_latched;
    volatile uint32_t button_event_drops;
    volatile uint32_t start_delay_ms;
    volatile uint32_t motor_armed;
    volatile uint32_t output_fault_mask;
    volatile uint32_t encoder_fault_mask;
    volatile uint32_t encoder_verified_mask;
    volatile uint32_t left_invalid_transitions;
    ChassisTestWheel left;
    ChassisTestWheel right;
    ChassisTestLineSensor line;
    ChassisTestMc02Link mc02;
    ChassisTestQuestionTimer timer;
} ChassisTestControl;

extern ChassisTestControl g_test;

void TestMode_Init(void);
void TestMode_Tick(void);
void TestMode_Stop(void);
int TestMode_TakeButtonEvent(uint8_t *button_id, uint8_t *button_action);
int TestMode_IsEmergencyStopLatched(void);
int TestMode_IsButtonPanelOk(void);
int TestMode_IsPowerEnabled(void);
uint8_t TestMode_GetCommonMotionPhase(void);
int TestMode_SupervisorPrepareQuestion(uint8_t question_id);
int TestMode_SupervisorStartQuestion(
    uint8_t question_id, uint32_t timeout_ms);
void TestMode_SupervisorStop(void);
int TestMode_IsRunActive(void);
int TestMode_RequestBluetoothStart(void);
int TestMode_RequestBluetoothLineFollow(void);
int TestMode_RequestBluetoothLineLap(void);
void TestMode_BluetoothKeepalive(void);

#endif
