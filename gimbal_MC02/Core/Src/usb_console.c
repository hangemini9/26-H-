#include "usb_console.h"
#include "usbd_cdc_if.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#define USB_RX_RING_SIZE 512U
#define USB_TX_BUFFER_SIZE 320U

static volatile uint16_t s_rx_write;
static volatile uint16_t s_rx_read;
static uint8_t s_rx_ring[USB_RX_RING_SIZE];
static uint8_t s_tx_buffer[USB_TX_BUFFER_SIZE];
static uint32_t s_rx_bytes;
static uint32_t s_rx_overflow;
static uint32_t s_tx_drop;

static uint8_t tx_is_ready(void)
{
    extern USBD_HandleTypeDef hUsbDeviceHS;
    USBD_CDC_HandleTypeDef *cdc;

    if (hUsbDeviceHS.pClassData == NULL)
    {
        return 0U;
    }
    cdc = (USBD_CDC_HandleTypeDef *)hUsbDeviceHS.pClassData;
    return (cdc->TxState == 0U) ? 1U : 0U;
}

void usb_console_init(void)
{
    s_rx_write = 0U;
    s_rx_read = 0U;
    s_rx_bytes = 0U;
    s_rx_overflow = 0U;
    s_tx_drop = 0U;
}

void usb_console_rx_isr(const uint8_t *data, uint32_t length)
{
    uint32_t index;

    for (index = 0U; index < length; index++)
    {
        uint16_t next = (uint16_t)((s_rx_write + 1U) % USB_RX_RING_SIZE);
        s_rx_bytes++;
        if (next == s_rx_read)
        {
            s_rx_overflow++;
            continue;
        }
        s_rx_ring[s_rx_write] = data[index];
        s_rx_write = next;
    }
}

uint8_t usb_console_get_line(char *line, uint16_t capacity)
{
    uint16_t cursor;
    uint16_t length;

    if ((line == NULL) || (capacity < 2U))
    {
        return 0U;
    }

    cursor = s_rx_read;
    length = 0U;
    while (cursor != s_rx_write)
    {
        uint8_t byte = s_rx_ring[cursor];
        cursor = (uint16_t)((cursor + 1U) % USB_RX_RING_SIZE);

        if ((byte == '\r') || (byte == '\n'))
        {
            s_rx_read = cursor;
            if (length == 0U)
            {
                continue;
            }
            line[length] = '\0';
            return 1U;
        }

        if (length < (uint16_t)(capacity - 1U))
        {
            line[length++] = (char)byte;
        }
        else
        {
            /* Discard an overlong line through its delimiter. */
            s_rx_overflow++;
        }
    }
    return 0U;
}

uint8_t usb_console_write(const char *text)
{
    size_t length;

    if (text == NULL)
    {
        return 0U;
    }
    if (tx_is_ready() == 0U)
    {
        s_tx_drop++;
        return 0U;
    }

    length = strlen(text);
    if (length >= USB_TX_BUFFER_SIZE)
    {
        length = USB_TX_BUFFER_SIZE - 1U;
    }
    memcpy(s_tx_buffer, text, length);

    if (CDC_Transmit_HS(s_tx_buffer, (uint16_t)length) != USBD_OK)
    {
        s_tx_drop++;
        return 0U;
    }
    return 1U;
}

uint8_t usb_console_write_bytes(const uint8_t *data, uint16_t length)
{
    if ((data == NULL) || (length == 0U) ||
        (length > USB_TX_BUFFER_SIZE))
    {
        return 0U;
    }
    if (tx_is_ready() == 0U)
    {
        return 0U;
    }

    memcpy(s_tx_buffer, data, length);
    if (CDC_Transmit_HS(s_tx_buffer, length) != USBD_OK)
    {
        return 0U;
    }
    return 1U;
}

void usb_console_discard_rx(void)
{
    s_rx_read = s_rx_write;
}

uint8_t usb_console_printf(const char *format, ...)
{
    int length;
    va_list arguments;

    if (format == NULL)
    {
        return 0U;
    }
    if (tx_is_ready() == 0U)
    {
        s_tx_drop++;
        return 0U;
    }

    va_start(arguments, format);
    length = vsnprintf((char *)s_tx_buffer,
                       USB_TX_BUFFER_SIZE,
                       format,
                       arguments);
    va_end(arguments);

    if (length < 0)
    {
        return 0U;
    }
    if (length >= (int)USB_TX_BUFFER_SIZE)
    {
        length = (int)USB_TX_BUFFER_SIZE - 1;
    }

    if (CDC_Transmit_HS(s_tx_buffer, (uint16_t)length) != USBD_OK)
    {
        s_tx_drop++;
        return 0U;
    }
    return 1U;
}

uint32_t usb_console_rx_bytes(void)
{
    return s_rx_bytes;
}

uint32_t usb_console_rx_overflow(void)
{
    return s_rx_overflow;
}

uint32_t usb_console_tx_drop(void)
{
    return s_tx_drop;
}
