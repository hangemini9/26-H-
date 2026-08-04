#include "imu.h"

ImuState g_imu;

void Imu_Init(void)
{
    g_imu.enabled = 0U;
    g_imu.ready = 0U;
    g_imu.sample_count = 0U;
    g_imu.gyro_x_dps = 0.0f;
    g_imu.gyro_y_dps = 0.0f;
    g_imu.gyro_z_dps = 0.0f;
    g_imu.accel_x_g = 0.0f;
    g_imu.accel_y_g = 0.0f;
    g_imu.accel_z_g = 0.0f;
}

void Imu_Tick(void)
{
    /* Intentionally empty until the ICM42688 bus and interrupt pin exist. */
}
