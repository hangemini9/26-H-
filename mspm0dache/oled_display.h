#ifndef OLED_DISPLAY_H
#define OLED_DISPLAY_H

#include <stdint.h>

void OledDisplay_Init(void);
void OledDisplay_Tick(
    uint32_t elapsed_seconds, uint8_t power_enabled);
uint8_t OledDisplay_IsConnected(void);
uint8_t OledDisplay_GetAddress(void);
uint32_t OledDisplay_GetErrorCount(void);
uint32_t OledDisplay_GetRequestedSeconds(void);
uint32_t OledDisplay_GetDisplayedSeconds(void);
uint32_t OledDisplay_GetRenderCount(void);

#endif
