#include "motor.h"
#include "ti_msp_dl_config.h"

void GROUP1_IRQHandler(void)
{
    const uint32_t encoder_pins =
        GPIO_ENCODER_LEFT_ENCODER_LEFT_A_PIN |
        GPIO_ENCODER_LEFT_ENCODER_LEFT_B_PIN;

    for (uint32_t pass = 0U; pass < 4U; pass++) {
        uint32_t encoder_status = DL_GPIO_getEnabledInterruptStatus(
            GPIO_ENCODER_LEFT_PORT, encoder_pins);
        if (encoder_status == 0U) {
            break;
        }

        /*
         * Clear the pending flags before sampling AB. An edge arriving during
         * the sample then creates a fresh pending flag for the next pass.
         */
        DL_GPIO_clearInterruptStatus(
            GPIO_ENCODER_LEFT_PORT, encoder_status);
        Motor_LeftEncoderGPIOIRQ();
    }
}
