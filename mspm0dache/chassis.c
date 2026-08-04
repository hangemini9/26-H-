#include "chassis.h"
#include <float.h>
#include <stdint.h>

static int float_is_finite(float value)
{
    return value == value && value <= FLT_MAX && value >= -FLT_MAX;
}

static int clamp_int(int value, int minimum, int maximum)
{
    if (value < minimum) {
        return minimum;
    }
    if (value > maximum) {
        return maximum;
    }
    return value;
}

static float wheel_circumference_mm(void)
{
    return CHASSIS_PI * CHASSIS_WHEEL_DIAMETER_MM;
}

static float mm_per_second_to_rpm(float speed_mm_s)
{
    return speed_mm_s * 60.0f / wheel_circumference_mm();
}

void Chassis_SetVelocity(int v_mm_s, int w_mrad_s)
{
    v_mm_s = clamp_int(v_mm_s,
        -CHASSIS_MAX_LINEAR_MM_S, CHASSIS_MAX_LINEAR_MM_S);
    w_mrad_s = clamp_int(w_mrad_s,
        -CHASSIS_MAX_ANGULAR_MRAD_S, CHASSIS_MAX_ANGULAR_MRAD_S);

    if (v_mm_s == 0 && w_mrad_s == 0) {
        Chassis_Stop();
        return;
    }

    float linear_mm_s = (float) v_mm_s;
    float angular_rad_s = (float) w_mrad_s * 0.001f;
    float half_track_mm = CHASSIS_DRIVE_TRACK_MM * 0.5f;
    float left_mm_s = linear_mm_s - angular_rad_s * half_track_mm;
    float right_mm_s = linear_mm_s + angular_rad_s * half_track_mm;

    Motor_SetTargetRPM(WHEEL_PHYS_LEFT,
        mm_per_second_to_rpm(left_mm_s));
    Motor_SetTargetRPM(WHEEL_PHYS_RIGHT,
        mm_per_second_to_rpm(right_mm_s));
}

void Chassis_Stop(void)
{
    Motor_Stop();
}

void Chassis_ResetOdom(void)
{
    Motor_ResetAllEncoders();
}

int64_t Chassis_GetEncLeft(void)
{
    return Motor_GetTotalCount(WHEEL_PHYS_LEFT);
}

int64_t Chassis_GetEncRight(void)
{
    return Motor_GetTotalCount(WHEEL_PHYS_RIGHT);
}

float Chassis_CountsToMm(Motor_ID id, int64_t counts)
{
    uint32_t counts_per_rev = Motor_GetCountsPerWheelRev(id);
    if (counts_per_rev == 0U) {
        return 0.0f;
    }
    return (float) counts * wheel_circumference_mm() /
        (float) counts_per_rev;
}

int64_t Chassis_MmToCounts(Motor_ID id, float distance_mm)
{
    uint32_t counts_per_rev = Motor_GetCountsPerWheelRev(id);
    if (!float_is_finite(distance_mm) || counts_per_rev == 0U) {
        return 0;
    }
    float counts = distance_mm * (float) counts_per_rev /
        wheel_circumference_mm();
    if (!float_is_finite(counts) ||
        counts >= (float) INT64_MAX ||
        counts <= (float) INT64_MIN) {
        return 0;
    }
    return (int64_t) (counts >= 0.0f ? counts + 0.5f : counts - 0.5f);
}

float Chassis_GetDistanceLeftMm(void)
{
    return Chassis_CountsToMm(MOTOR_LEFT, Chassis_GetEncLeft());
}

float Chassis_GetDistanceRightMm(void)
{
    return Chassis_CountsToMm(MOTOR_RIGHT, Chassis_GetEncRight());
}

float Chassis_GetAverageDistanceMm(void)
{
    return 0.5f *
        (Chassis_GetDistanceLeftMm() + Chassis_GetDistanceRightMm());
}

float Chassis_GetEstimatedYawDeg(void)
{
    float delta_mm =
        Chassis_GetDistanceRightMm() - Chassis_GetDistanceLeftMm();
    float yaw_rad = delta_mm / CHASSIS_DRIVE_TRACK_MM;
    return yaw_rad * (180.0f / CHASSIS_PI);
}

float Chassis_TurnDegToWheelTravelMm(float angle_deg)
{
    if (!float_is_finite(angle_deg)) {
        return 0.0f;
    }
    return CHASSIS_PI * CHASSIS_DRIVE_TRACK_MM * angle_deg / 360.0f;
}
