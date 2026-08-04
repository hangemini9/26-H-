#ifndef TI_LINK_H
#define TI_LINK_H

#include <stdint.h>

void ti_link_init(uint32_t now_ms);
void ti_link_poll(uint32_t now_ms);
void ti_link_uart_irq_handler(void);
uint16_t ti_link_crc16(const uint8_t *data, uint16_t length);
uint32_t ti_link_run_id(void);
uint8_t ti_link_question_id(void);
uint8_t ti_link_chassis_online(void);
uint8_t ti_link_supervisor_state(void);
float ti_link_chassis_acceleration_m_s2(void);

#endif
