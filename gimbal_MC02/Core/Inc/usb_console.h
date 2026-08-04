#ifndef USB_CONSOLE_H
#define USB_CONSOLE_H

#include <stdint.h>

void usb_console_init(void);
void usb_console_rx_isr(const uint8_t *data, uint32_t length);
uint8_t usb_console_get_line(char *line, uint16_t capacity);
uint8_t usb_console_write(const char *text);
uint8_t usb_console_write_bytes(const uint8_t *data, uint16_t length);
uint8_t usb_console_printf(const char *format, ...);
void usb_console_discard_rx(void);
uint32_t usb_console_rx_bytes(void);
uint32_t usb_console_rx_overflow(void);
uint32_t usb_console_tx_drop(void);

#endif
