#ifndef PLATFORM_H
#define PLATFORM_H

#include <stdint.h>

void Platform_InitTick(void);
uint32_t Platform_Millis(void);
void Platform_DelayMs(uint32_t delay_ms);
uint32_t Platform_EnterCritical(void);
void Platform_ExitCritical(uint32_t primask);

#endif
