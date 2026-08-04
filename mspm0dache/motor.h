#ifndef MOTOR_H
#define MOTOR_H

#include <stdint.h>

#include "chassis_config.h"

typedef enum {
    MOTOR_RIGHT = 0,
    MOTOR_LEFT = 1,
    MOTOR_NUM
} Motor_ID;

typedef struct {
    uint32_t last_count;
    uint32_t current_count;
    int32_t delta_count;
    int64_t total_count;
    float raw_speed_rpm;
    float speed_rpm;
    float speed_radps;
} Encoder_TypeDef;

typedef struct {
    float kp;
    float ki;
    float kd;
    float integral;
    float prev_error;
    float integral_max;
    float out_max;
    float out_min;
    float output;
} PID_TypeDef;

typedef struct {
    Encoder_TypeDef encoder;
    PID_TypeDef pid;
    float target_rpm;
    float current_rpm;
    int16_t requested_pwm;
    int16_t pwm;
    uint8_t closed_loop;
} Motor_TypeDef;

void Motor_Init(void);
void Motor_ControlTick(void);

/*
 * Non-zero motor commands are accepted only while armed. Disarm immediately
 * changes both PWM pins to GPIO-low, drives both DIR pins low, and clears all
 * closed-loop targets.
 */
void Motor_Arm(void);
void Motor_Disarm(void);
int Motor_IsArmed(void);

void Motor_SetPWM(Motor_ID id, int16_t pwm);
void Motor_SetTargetRPM(Motor_ID id, float rpm);
void Motor_Stop(void);
void Motor_PID_SetParams(Motor_ID id, float kp, float ki, float kd);

float Motor_GetSpeedRPM(Motor_ID id);
float Motor_GetSpeedRadPS(Motor_ID id);
int64_t Motor_GetTotalCount(Motor_ID id);
int16_t Motor_GetAppliedPWM(Motor_ID id);
float Motor_GetPIDIntegral(Motor_ID id);
float Motor_GetPIDOutput(Motor_ID id);
uint32_t Motor_GetCountsPerWheelRev(Motor_ID id);
void Motor_ResetEncoder(Motor_ID id);
void Motor_ResetAllEncoders(void);

/* Called by GROUP1_IRQHandler for the software-decoded left encoder. */
void Motor_LeftEncoderGPIOIRQ(void);
uint32_t Motor_GetLeftEncoderInvalidTransitions(void);

#endif
