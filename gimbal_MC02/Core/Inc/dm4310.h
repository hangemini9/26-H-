#ifndef DM4310_H
#define DM4310_H

#include "fdcan.h"
#include <stdint.h>

typedef struct
{
    uint8_t initialized;
    uint8_t online;
    uint8_t motor_id;
    uint8_t error;
    uint8_t mos_temperature_c;
    uint8_t rotor_temperature_c;
    float position_rad;
    float velocity_rad_s;
    float torque_nm;
    uint32_t last_rx_ms;
    uint32_t rx_count;
    uint32_t tx_count;
    uint32_t tx_error_count;
    uint32_t rx_invalid_count;
    uint32_t hal_error;
} Dm4310Status;

void dm4310_init(FDCAN_HandleTypeDef *hfdcan, uint32_t now_ms);
void dm4310_poll(uint32_t now_ms);
HAL_StatusTypeDef dm4310_query(void);
HAL_StatusTypeDef dm4310_enable(void);
HAL_StatusTypeDef dm4310_disable(void);
HAL_StatusTypeDef dm4310_command_position_speed(float position_rad,
                                                float velocity_rad_s);
const Dm4310Status *dm4310_status(void);

#endif
