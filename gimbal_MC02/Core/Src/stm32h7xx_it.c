#include "main.h"
#include "stm32h7xx_it.h"
#include "fdcan.h"
#include "ti_link.h"

extern PCD_HandleTypeDef hpcd_USB_OTG_HS;
extern TIM_HandleTypeDef htim23;

void NMI_Handler(void)
{
}

void HardFault_Handler(void)
{
    while (1)
    {
    }
}

void MemManage_Handler(void)
{
    while (1)
    {
    }
}

void BusFault_Handler(void)
{
    while (1)
    {
    }
}

void UsageFault_Handler(void)
{
    while (1)
    {
    }
}

void USART1_IRQHandler(void)
{
    ti_link_uart_irq_handler();
}

void SVC_Handler(void)
{
}

void DebugMon_Handler(void)
{
}

void PendSV_Handler(void)
{
}

void SysTick_Handler(void)
{
}

void FDCAN1_IT0_IRQHandler(void)
{
    HAL_FDCAN_IRQHandler(&hfdcan1);
}

void FDCAN1_IT1_IRQHandler(void)
{
    HAL_FDCAN_IRQHandler(&hfdcan1);
}

void OTG_HS_IRQHandler(void)
{
    HAL_PCD_IRQHandler(&hpcd_USB_OTG_HS);
}

void TIM23_IRQHandler(void)
{
    HAL_TIM_IRQHandler(&htim23);
}
