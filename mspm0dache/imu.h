#ifndef IMU_H
#define IMU_H

#include <stdint.h>

/*
 * ICM42688 placeholder. No bus or GPIO is assigned in the first firmware.
 * Future integration can keep this public state/API without changing main.c.
 */
#define ICM42688_ENABLED 0

typedef struct {
    volatile uint8_t enabled;
    volatile uint8_t ready;
    volatile uint32_t sample_count;
    volatile float gyro_x_dps;
    volatile float gyro_y_dps;
    volatile float gyro_z_dps;
    volatile float accel_x_g;
    volatile float accel_y_g;
    volatile float accel_z_g;
} ImuState;

extern ImuState g_imu;

void Imu_Init(void);
void Imu_Tick(void);

#endif
