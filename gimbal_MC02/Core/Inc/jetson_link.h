#ifndef JETSON_LINK_H
#define JETSON_LINK_H

#include <stdint.h>

void jetson_link_init(uint32_t now_ms);
void jetson_link_rx_isr(const uint8_t *data, uint32_t length);
void jetson_link_poll(uint32_t now_ms);
void jetson_link_send_event(
    uint32_t run_id, uint16_t event_code, uint16_t detail,
    uint32_t now_ms);
uint8_t jetson_link_online(void);
uint8_t jetson_link_ready(void);
uint32_t jetson_link_last_heartbeat_ms(void);
uint16_t jetson_link_crc16(const uint8_t *data, uint16_t length);

#endif
