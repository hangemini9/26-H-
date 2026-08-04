#include <stdint.h>

#include "imu.h"
#include "line_sensor.h"
#include "mc02_link.h"
#include "motor.h"
#include "platform.h"
#include "question_timer.h"
#include "test_mode.h"
#include "ti_msp_dl_config.h"

#define TEST_TASK_PERIOD_MS 10U

static int task_due(uint32_t now, uint32_t *last, uint32_t period)
{
    if ((uint32_t) (now - *last) < period) {
        return 0;
    }
    *last += period;
    if ((uint32_t) (now - *last) >= period) {
        *last = now;
    }
    return 1;
}

int main(void)
{
    SYSCFG_DL_init();
    Platform_InitTick();

    Motor_Init();
    Imu_Init();
    LineSensor_Init();
    QuestionTimer_Init();
    TestMode_Init();
    MC02Link_Init();

    DL_GPIO_clearInterruptStatus(GPIO_ENCODER_LEFT_PORT,
        GPIO_ENCODER_LEFT_ENCODER_LEFT_A_PIN |
            GPIO_ENCODER_LEFT_ENCODER_LEFT_B_PIN);
    NVIC_ClearPendingIRQ(GPIO_ENCODER_LEFT_INT_IRQN);
    NVIC_EnableIRQ(GPIO_ENCODER_LEFT_INT_IRQN);

    uint32_t now = Platform_Millis();
    uint32_t last_motor = now;
    uint32_t last_test = now;

    for (;;) {
        now = Platform_Millis();

        if (task_due(now, &last_motor, MOTOR_CTRL_PERIOD_MS)) {
            Motor_ControlTick();
        }
        if (task_due(now, &last_test, TEST_TASK_PERIOD_MS)) {
            LineSensor_Tick();
            TestMode_Tick();
            MC02Link_Tick();
            Imu_Tick();
            QuestionTimer_Tick();
        }

        __WFI();
    }
}
