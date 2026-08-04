#include "platform.h"

#include "ti_msp_dl_config.h"

static volatile uint32_t s_millis;

void Platform_InitTick(void)
{
    s_millis = 0U;
    (void) SysTick_Config(CPUCLK_FREQ / 1000U);
}

uint32_t Platform_Millis(void)
{
    return s_millis;
}

void Platform_DelayMs(uint32_t delay_ms)
{
    uint32_t start = Platform_Millis();
    while ((uint32_t) (Platform_Millis() - start) < delay_ms) {
        __WFI();
    }
}

uint32_t Platform_EnterCritical(void)
{
    uint32_t primask = __get_PRIMASK();
    __disable_irq();
    return primask;
}

void Platform_ExitCritical(uint32_t primask)
{
    if (primask == 0U) {
        __enable_irq();
    }
}

void SysTick_Handler(void)
{
    s_millis++;
}
