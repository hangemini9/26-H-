#ifndef CHASSIS_H
#define CHASSIS_H

#include <stdint.h>

#include "motor.h"

#define WHEEL_PHYS_LEFT  MOTOR_LEFT
#define WHEEL_PHYS_RIGHT MOTOR_RIGHT

void Chassis_SetVelocity(int v_mm_s, int w_mrad_s);
void Chassis_Stop(void);
void Chassis_ResetOdom(void);

int64_t Chassis_GetEncLeft(void);
int64_t Chassis_GetEncRight(void);
float Chassis_CountsToMm(Motor_ID id, int64_t counts);
int64_t Chassis_MmToCounts(Motor_ID id, float distance_mm);
float Chassis_GetDistanceLeftMm(void);
float Chassis_GetDistanceRightMm(void);
float Chassis_GetAverageDistanceMm(void);
float Chassis_GetEstimatedYawDeg(void);
float Chassis_TurnDegToWheelTravelMm(float angle_deg);

#endif
