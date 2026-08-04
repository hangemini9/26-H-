#ifndef GIMBAL_APP_H
#define GIMBAL_APP_H

#include "vision_link.h"
#include <stdint.h>

typedef enum
{
    GIMBAL_STATE_BOOT = 0,
    GIMBAL_STATE_SAFE_IDLE = 1,
    GIMBAL_STATE_OBSERVE = 2,
    GIMBAL_STATE_HOLD = 3,
    GIMBAL_STATE_STEP = 4,
    GIMBAL_STATE_SWEEP = 5,
    GIMBAL_STATE_FAULT = 6,
    GIMBAL_STATE_COMPETITION_PREP = 7,
    GIMBAL_STATE_COMPETITION = 8,
    GIMBAL_STATE_DELAY_TEST_PREP = 9,
    GIMBAL_STATE_DELAY_TEST = 10
} GimbalState;

typedef enum
{
    GIMBAL_FAULT_NONE = 0,
    GIMBAL_FAULT_CAN_INIT = 1,
    GIMBAL_FAULT_CAN_TX = 2,
    GIMBAL_FAULT_FEEDBACK_TIMEOUT = 3,
    GIMBAL_FAULT_MOTOR_REPORTED = 4,
    GIMBAL_FAULT_BAD_COMMAND = 5,
    GIMBAL_FAULT_TI_LINK = 6,
    GIMBAL_FAULT_JETSON_LINK = 7,
    GIMBAL_FAULT_VISION_LOST = 8
} GimbalFault;

typedef enum
{
    GIMBAL_REQUEST_NONE = 0,
    GIMBAL_REQUEST_STOP = 1,
    GIMBAL_REQUEST_OBSERVE = 2,
    GIMBAL_REQUEST_HOLD = 3,
    GIMBAL_REQUEST_STEP = 4,
    GIMBAL_REQUEST_SWEEP = 5,
    GIMBAL_REQUEST_CLEAR_FAULT = 6
} GimbalRequest;

typedef enum
{
    GIMBAL_DELAY_REQUEST_NONE = 0,
    GIMBAL_DELAY_REQUEST_PREPARE = 1,
    GIMBAL_DELAY_REQUEST_VALIDATE_EMPTY = 2,
    GIMBAL_DELAY_REQUEST_ACK_MECHANICAL = 3,
    GIMBAL_DELAY_REQUEST_MEASURE = 4,
    GIMBAL_DELAY_REQUEST_STOP = 5
} GimbalDelayTestRequest;

typedef enum
{
    GIMBAL_TI_STOP_NORMAL = 0,
    GIMBAL_TI_STOP_LINK_FAULT = 1,
    GIMBAL_TI_STOP_PRESERVE_FAULT = 2
} GimbalTiStopMode;

/*
 * One compact structure for SEGGER Ozone Watch.
 *
 * Safe debugger workflow:
 *   1. Set arm_key to GIMBAL_DEBUG_ARM_KEY.
 *   2. Set request_param_tenths_deg if STEP/SWEEP is requested.
 *   3. Set request last.
 *
 * The firmware consumes and clears both arm_key and request.  STOP and
 * OBSERVE never require arm_key.  Do not halt the CPU during powered motion.
 */
typedef struct
{
    volatile uint32_t build_id;
    volatile uint32_t uptime_ms;
    volatile uint32_t arm_key;
    volatile uint32_t request;
    volatile int32_t request_param_tenths_deg;

    volatile uint32_t state;
    volatile uint32_t fault;
    volatile uint32_t state_elapsed_ms;
    volatile uint32_t arm_remaining_ms;
    volatile uint32_t power_output_enabled;
    volatile uint32_t power_arm_remaining_ms;
    volatile uint32_t power_on_remaining_ms;
    volatile uint32_t calrun_phase;
    volatile uint32_t delay_test_enabled;
    volatile uint32_t delay_test_key;
    volatile uint32_t delay_test_request;
    volatile uint32_t delay_test_phase;
    volatile uint32_t delay_test_arm_remaining_ms;
    volatile uint32_t delay_test_validated;
    volatile uint32_t delay_test_mechanical_ack;
    volatile uint32_t delay_test_measurement_active;
    volatile uint32_t delay_test_pulse_count;
    volatile uint32_t delay_test_measurement_count;
    volatile uint32_t delay_test_denied_count;
    volatile uint32_t delay_test_last_denied_request;
    volatile uint32_t delay_test_last_pulse_start_ms;
    volatile uint32_t delay_test_stable_ready;
    volatile uint32_t delay_test_stable_elapsed_ms;
    volatile uint32_t delay_test_stable_span_um;
    volatile uint32_t delay_test_stable_sample_count;
    volatile int32_t delay_test_fixed_pipe_angle_mdeg;
    volatile float delay_test_fixed_motor_speed_rad_s;

    volatile uint32_t level_zero_valid;
    volatile float level_zero_position_rad;
    volatile float motor_offset_from_level_rad;
    volatile float neutral_position_rad;
    volatile float requested_position_rad;
    volatile float command_position_rad;
    volatile float feedback_position_rad;
    volatile float feedback_velocity_rad_s;
    volatile float feedback_torque_nm;
    volatile uint32_t feedback_age_ms;
    volatile uint32_t motor_error;
    volatile uint32_t motor_mos_temperature_c;
    volatile uint32_t motor_rotor_temperature_c;

    volatile uint32_t can_rx_count;
    volatile uint32_t can_tx_count;
    volatile uint32_t can_tx_error_count;
    volatile uint32_t usb_rx_bytes;
    volatile uint32_t usb_rx_overflow;
    volatile uint32_t usb_tx_drop;

    volatile int32_t vision_position_um;
    volatile int32_t vision_velocity_um_s;
    volatile uint32_t vision_processing_latency_us;
    volatile uint32_t vision_age_ms;
    volatile uint32_t vision_flags;
    volatile uint32_t vision_closed_loop_enabled;
    volatile uint32_t competition_run_id;
    volatile uint32_t competition_question_id;
    volatile uint32_t competition_phase;
    volatile int32_t competition_target_um;
    volatile int32_t command_pipe_angle_mdeg;
    volatile uint32_t target_settle_ms;
    volatile uint32_t q2_breakaway_active;
    volatile uint32_t q2_breakaway_remaining_ms;
    volatile uint32_t q2_breakaway_count;
    volatile int32_t q2_breakaway_pipe_angle_mdeg;
    volatile uint32_t q2_breakaway_used_this_leg;
    volatile uint32_t q2_stall_elapsed_ms;
    volatile int32_t q2_stall_span_um;
    volatile uint32_t control_total_delay_ms;
    volatile uint32_t control_prediction_horizon_ms;
    volatile int32_t control_predicted_position_um;
    volatile int32_t control_velocity_um_s;
    volatile uint32_t control_velocity_window_ms;
    volatile uint32_t q2_stationary_ms;
    volatile uint32_t q2_final_capture_active;
    volatile float control_chassis_forward_to_x_sign;
    volatile float control_chassis_accel_m_s2;
    volatile int32_t control_chassis_feedforward_mdeg;
    volatile float control_kp_s2;
    volatile float control_kd_s;
    volatile float pipe_per_motor_ratio;
    volatile float max_pipe_angle_deg;
    volatile float max_motor_slew_rad_s;

    volatile uint32_t jetson_online;
    volatile uint32_t jetson_pipeline_state;
    volatile uint32_t jetson_flags;
    volatile uint32_t jetson_heartbeat_age_ms;
    volatile uint32_t jetson_rx_frames;
    volatile uint32_t jetson_crc_errors;
    volatile uint32_t jetson_sequence_drops;
    volatile uint32_t jetson_tx_frames;
    volatile uint32_t jetson_tx_drops;

    volatile uint32_t ti_online;
    volatile uint32_t ti_supervisor_state;
    volatile uint32_t ti_chassis_state;
    volatile uint32_t ti_chassis_fault;
    volatile uint32_t ti_chassis_flags;
    volatile uint32_t ti_run_id;
    volatile uint32_t ti_question_id;
    volatile uint32_t ti_button_mask;
    volatile uint32_t ti_last_button_id;
    volatile uint32_t ti_emergency_stop_latched;
    volatile uint32_t ti_unsupported_question_id;
    volatile int32_t ti_left_rpm_x10;
    volatile int32_t ti_right_rpm_x10;
    volatile int32_t ti_chassis_accel_mm_s2;
    volatile uint32_t ti_chassis_accel_tail_remaining_ms;
    volatile uint32_t ti_route_complete_waiting;
    volatile uint32_t ti_route_hold_remaining_ms;
    volatile uint32_t ti_ball_settle_elapsed_ms;
    volatile uint32_t ti_rx_frames;
    volatile uint32_t ti_crc_errors;
    volatile uint32_t ti_tx_frames;
    volatile uint32_t ti_tx_drops;
} GimbalDebug;

extern volatile GimbalDebug g_gimbal_debug;

void gimbal_app_init(uint32_t now_ms);
void gimbal_app_poll(uint32_t now_ms);
void gimbal_app_on_vision_sample(const VisionSample *sample);
uint8_t gimbal_app_begin_question(
    uint32_t run_id, uint8_t question_id, uint32_t now_ms);
uint8_t gimbal_app_question_ready(uint32_t run_id);
void gimbal_app_start_question(uint32_t run_id, uint32_t now_ms);
uint8_t gimbal_app_question_complete(uint32_t run_id);
uint8_t gimbal_app_question_faulted(uint32_t run_id);
uint8_t gimbal_app_ball_within_route_settle_limits(uint32_t now_ms);
void gimbal_app_complete_question(uint32_t run_id, uint32_t now_ms);
void gimbal_app_stop_question(
    uint32_t run_id, uint8_t faulted, uint32_t now_ms);
uint8_t gimbal_app_system_state(void);
int32_t gimbal_app_target_position_um(void);
int32_t gimbal_app_command_pipe_angle_mdeg(void);
uint16_t gimbal_app_vision_age_ms(uint32_t now_ms);
uint8_t gimbal_app_motor_enabled(void);
uint8_t gimbal_app_stop_latched(void);
void gimbal_app_force_ti_safe_stop(
    uint32_t now_ms, GimbalTiStopMode stop_mode);

#endif
