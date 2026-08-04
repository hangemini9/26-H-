#ifndef LINE_SENSOR_H
#define LINE_SENSOR_H

#include <stdint.h>

#define LINE_SENSOR_CHANNEL_COUNT 8U

typedef struct {
    uint32_t connected;
    uint32_t sample_sequence;
    uint32_t error_count;
    uint8_t state;
    uint8_t active_count;
    uint8_t line_lost;
    int32_t position_x1000;
    uint16_t analog[LINE_SENSOR_CHANNEL_COUNT];
    uint16_t threshold[LINE_SENSOR_CHANNEL_COUNT];
} LineSensorData;

void LineSensor_Init(void);
void LineSensor_Tick(void);
const LineSensorData *LineSensor_GetData(void);

#endif
