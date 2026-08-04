/*
 * Copyright (c) 2023, Texas Instruments Incorporated - http://www.ti.com
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 *  ============ ti_msp_dl_config.h =============
 *  Configured MSPM0 DriverLib module declarations
 *
 *  DO NOT EDIT - This file is generated for the MSPM0G350X
 *  by the SysConfig tool.
 */
#ifndef ti_msp_dl_config_h
#define ti_msp_dl_config_h

#define CONFIG_MSPM0G350X
#define CONFIG_MSPM0G3507

#if defined(__ti_version__) || defined(__TI_COMPILER_VERSION__)
#define SYSCONFIG_WEAK __attribute__((weak))
#elif defined(__IAR_SYSTEMS_ICC__)
#define SYSCONFIG_WEAK __weak
#elif defined(__GNUC__)
#define SYSCONFIG_WEAK __attribute__((weak))
#endif

#include <ti/devices/msp/msp.h>
#include <ti/driverlib/driverlib.h>
#include <ti/driverlib/m0p/dl_core.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 *  ======== SYSCFG_DL_init ========
 *  Perform all required MSP DL initialization
 *
 *  This function should be called once at a point before any use of
 *  MSP DL.
 */


/* clang-format off */

#define POWER_STARTUP_DELAY                                                (16)



#define CPUCLK_FREQ                                                     32000000



/* Defines for MOTOR_PWM */
#define MOTOR_PWM_INST                                                     TIMG0
#define MOTOR_PWM_INST_IRQHandler                               TIMG0_IRQHandler
#define MOTOR_PWM_INST_INT_IRQN                                 (TIMG0_INT_IRQn)
#define MOTOR_PWM_INST_CLK_FREQ                                         32000000
/* GPIO defines for channel 0 */
#define GPIO_MOTOR_PWM_C0_PORT                                             GPIOA
#define GPIO_MOTOR_PWM_C0_PIN                                     DL_GPIO_PIN_12
#define GPIO_MOTOR_PWM_C0_IOMUX                                  (IOMUX_PINCM34)
#define GPIO_MOTOR_PWM_C0_IOMUX_FUNC                 IOMUX_PINCM34_PF_TIMG0_CCP0
#define GPIO_MOTOR_PWM_C0_IDX                                DL_TIMER_CC_0_INDEX
/* GPIO defines for channel 1 */
#define GPIO_MOTOR_PWM_C1_PORT                                             GPIOA
#define GPIO_MOTOR_PWM_C1_PIN                                     DL_GPIO_PIN_13
#define GPIO_MOTOR_PWM_C1_IOMUX                                  (IOMUX_PINCM35)
#define GPIO_MOTOR_PWM_C1_IOMUX_FUNC                 IOMUX_PINCM35_PF_TIMG0_CCP1
#define GPIO_MOTOR_PWM_C1_IDX                                DL_TIMER_CC_1_INDEX




/* Defines for QEI_RIGHT */
#define QEI_RIGHT_INST                                                     TIMG8
#define QEI_RIGHT_INST_IRQHandler                               TIMG8_IRQHandler
#define QEI_RIGHT_INST_INT_IRQN                                 (TIMG8_INT_IRQn)
/* Pin configuration defines for QEI_RIGHT PHA Pin */
#define GPIO_QEI_RIGHT_PHA_PORT                                            GPIOB
#define GPIO_QEI_RIGHT_PHA_PIN                                     DL_GPIO_PIN_6
#define GPIO_QEI_RIGHT_PHA_IOMUX                                 (IOMUX_PINCM23)
#define GPIO_QEI_RIGHT_PHA_IOMUX_FUNC                IOMUX_PINCM23_PF_TIMG8_CCP0
/* Pin configuration defines for QEI_RIGHT PHB Pin */
#define GPIO_QEI_RIGHT_PHB_PORT                                            GPIOB
#define GPIO_QEI_RIGHT_PHB_PIN                                     DL_GPIO_PIN_7
#define GPIO_QEI_RIGHT_PHB_IOMUX                                 (IOMUX_PINCM24)
#define GPIO_QEI_RIGHT_PHB_IOMUX_FUNC                IOMUX_PINCM24_PF_TIMG8_CCP1



/* Defines for LINE_SENSOR_I2C */
#define LINE_SENSOR_I2C_INST                                                I2C1
#define LINE_SENSOR_I2C_INST_IRQHandler                          I2C1_IRQHandler
#define LINE_SENSOR_I2C_INST_INT_IRQN                              I2C1_INT_IRQn
#define LINE_SENSOR_I2C_BUS_SPEED_HZ                                      100000
#define GPIO_LINE_SENSOR_I2C_SDA_PORT                                      GPIOB
#define GPIO_LINE_SENSOR_I2C_SDA_PIN                               DL_GPIO_PIN_3
#define GPIO_LINE_SENSOR_I2C_IOMUX_SDA                           (IOMUX_PINCM16)
#define GPIO_LINE_SENSOR_I2C_IOMUX_SDA_FUNC               IOMUX_PINCM16_PF_I2C1_SDA
#define GPIO_LINE_SENSOR_I2C_SCL_PORT                                      GPIOB
#define GPIO_LINE_SENSOR_I2C_SCL_PIN                               DL_GPIO_PIN_2
#define GPIO_LINE_SENSOR_I2C_IOMUX_SCL                           (IOMUX_PINCM15)
#define GPIO_LINE_SENSOR_I2C_IOMUX_SCL_FUNC               IOMUX_PINCM15_PF_I2C1_SCL


/* Defines for MC02_UART */
#define MC02_UART_INST                                                     UART1
#define MC02_UART_INST_FREQUENCY                                        32000000
#define MC02_UART_INST_IRQHandler                               UART1_IRQHandler
#define MC02_UART_INST_INT_IRQN                                   UART1_INT_IRQn
#define GPIO_MC02_UART_RX_PORT                                             GPIOA
#define GPIO_MC02_UART_TX_PORT                                             GPIOA
#define GPIO_MC02_UART_RX_PIN                                      DL_GPIO_PIN_9
#define GPIO_MC02_UART_TX_PIN                                      DL_GPIO_PIN_8
#define GPIO_MC02_UART_IOMUX_RX                                  (IOMUX_PINCM20)
#define GPIO_MC02_UART_IOMUX_TX                                  (IOMUX_PINCM19)
#define GPIO_MC02_UART_IOMUX_RX_FUNC                   IOMUX_PINCM20_PF_UART1_RX
#define GPIO_MC02_UART_IOMUX_TX_FUNC                   IOMUX_PINCM19_PF_UART1_TX
#define MC02_UART_BAUD_RATE                                             (115200)
#define MC02_UART_IBRD_32_MHZ_115200_BAUD                                   (17)
#define MC02_UART_FBRD_32_MHZ_115200_BAUD                                   (23)





/* Port definition for Pin Group GPIO_MOTOR */
#define GPIO_MOTOR_PORT                                                  (GPIOB)

/* Defines for RIGHT_DIR: GPIOB.4 with pinCMx 17 on package pin 52 */
#define GPIO_MOTOR_RIGHT_DIR_PIN                                 (DL_GPIO_PIN_4)
#define GPIO_MOTOR_RIGHT_DIR_IOMUX                               (IOMUX_PINCM17)
/* Defines for LEFT_DIR: GPIOB.1 with pinCMx 13 on package pin 48 */
#define GPIO_MOTOR_LEFT_DIR_PIN                                  (DL_GPIO_PIN_1)
#define GPIO_MOTOR_LEFT_DIR_IOMUX                                (IOMUX_PINCM13)
/* Port definition for Pin Group GPIO_ENCODER_LEFT */
#define GPIO_ENCODER_LEFT_PORT                                           (GPIOB)

/* Defines for ENCODER_LEFT_A: GPIOB.16 with pinCMx 33 on package pin 4 */
// pins affected by this interrupt request:["ENCODER_LEFT_A","ENCODER_LEFT_B"]
#define GPIO_ENCODER_LEFT_INT_IRQN                              (GPIOB_INT_IRQn)
#define GPIO_ENCODER_LEFT_INT_IIDX              (DL_INTERRUPT_GROUP1_IIDX_GPIOB)
#define GPIO_ENCODER_LEFT_ENCODER_LEFT_A_IIDX               (DL_GPIO_IIDX_DIO16)
#define GPIO_ENCODER_LEFT_ENCODER_LEFT_A_PIN                    (DL_GPIO_PIN_16)
#define GPIO_ENCODER_LEFT_ENCODER_LEFT_A_IOMUX                   (IOMUX_PINCM33)
/* Defines for ENCODER_LEFT_B: GPIOB.0 with pinCMx 12 on package pin 47 */
#define GPIO_ENCODER_LEFT_ENCODER_LEFT_B_IIDX                (DL_GPIO_IIDX_DIO0)
#define GPIO_ENCODER_LEFT_ENCODER_LEFT_B_PIN                     (DL_GPIO_PIN_0)
#define GPIO_ENCODER_LEFT_ENCODER_LEFT_B_IOMUX                   (IOMUX_PINCM12)
/* Port definition for Pin Group GPIO_BUTTON_PANEL_A */
#define GPIO_BUTTON_PANEL_A_PORT                                         (GPIOA)

/* Defines for BUTTON1: GPIOA.10 with pinCMx 21 on package pin 56 */
#define GPIO_BUTTON_PANEL_A_BUTTON1_PIN                         (DL_GPIO_PIN_10)
#define GPIO_BUTTON_PANEL_A_BUTTON1_IOMUX                        (IOMUX_PINCM21)
/* Defines for BUTTON2: GPIOA.11 with pinCMx 22 on package pin 57 */
#define GPIO_BUTTON_PANEL_A_BUTTON2_PIN                         (DL_GPIO_PIN_11)
#define GPIO_BUTTON_PANEL_A_BUTTON2_IOMUX                        (IOMUX_PINCM22)
/* Defines for BUTTON5: GPIOA.31 with pinCMx 6 on package pin 39 */
#define GPIO_BUTTON_PANEL_A_BUTTON5_PIN                         (DL_GPIO_PIN_31)
#define GPIO_BUTTON_PANEL_A_BUTTON5_IOMUX                         (IOMUX_PINCM6)
/* Defines for BUTTON6: GPIOA.28 with pinCMx 3 on package pin 35 */
#define GPIO_BUTTON_PANEL_A_BUTTON6_PIN                         (DL_GPIO_PIN_28)
#define GPIO_BUTTON_PANEL_A_BUTTON6_IOMUX                         (IOMUX_PINCM3)
/* Port definition for Pin Group GPIO_BUTTON_PANEL_B */
#define GPIO_BUTTON_PANEL_B_PORT                                         (GPIOB)

/* Defines for BUTTON3: GPIOB.13 with pinCMx 30 on package pin 1 */
#define GPIO_BUTTON_PANEL_B_BUTTON3_PIN                         (DL_GPIO_PIN_13)
#define GPIO_BUTTON_PANEL_B_BUTTON3_IOMUX                        (IOMUX_PINCM30)
/* Defines for BUTTON4: GPIOB.20 with pinCMx 48 on package pin 19 */
#define GPIO_BUTTON_PANEL_B_BUTTON4_PIN                         (DL_GPIO_PIN_20)
#define GPIO_BUTTON_PANEL_B_BUTTON4_IOMUX                        (IOMUX_PINCM48)
/* Defines for BUTTON7: GPIOB.12 with pinCMx 29 on package pin 64 */
#define GPIO_BUTTON_PANEL_B_BUTTON7_PIN                         (DL_GPIO_PIN_12)
#define GPIO_BUTTON_PANEL_B_BUTTON7_IOMUX                        (IOMUX_PINCM29)
/* Defines for BUTTON8: GPIOB.15 with pinCMx 32 on package pin 3 */
#define GPIO_BUTTON_PANEL_B_BUTTON8_PIN                         (DL_GPIO_PIN_15)
#define GPIO_BUTTON_PANEL_B_BUTTON8_IOMUX                        (IOMUX_PINCM32)


/* clang-format on */

void SYSCFG_DL_init(void);
void SYSCFG_DL_initPower(void);
void SYSCFG_DL_GPIO_init(void);
void SYSCFG_DL_SYSCTL_init(void);
void SYSCFG_DL_MOTOR_PWM_init(void);
void SYSCFG_DL_QEI_RIGHT_init(void);
void SYSCFG_DL_LINE_SENSOR_I2C_init(void);
void SYSCFG_DL_MC02_UART_init(void);


bool SYSCFG_DL_saveConfiguration(void);
bool SYSCFG_DL_restoreConfiguration(void);

#ifdef __cplusplus
}
#endif

#endif /* ti_msp_dl_config_h */
