#include "bluetooth.h"

#include <stdint.h>
#include <string.h>

#include "platform.h"
#include "test_mode.h"
#include "ti_msp_dl_config.h"

#define RX_BUFFER_SIZE          64U
#define TX_BUFFER_SIZE         128U
#define COMMAND_BUFFER_SIZE     32U
#define BLUETOOTH_ARM_WINDOW_MS 5000U

static volatile uint8_t s_rx_buffer[RX_BUFFER_SIZE];
static volatile uint8_t s_rx_head;
static volatile uint8_t s_rx_tail;
static volatile uint8_t s_rx_overflow;

static volatile uint8_t s_tx_buffer[TX_BUFFER_SIZE];
static volatile uint8_t s_tx_head;
static volatile uint8_t s_tx_tail;

static char s_command[COMMAND_BUFFER_SIZE];
static uint8_t s_command_length;
static uint8_t s_command_discard;
static uint8_t s_bluetooth_armed;
static uint32_t s_bluetooth_arm_ms;

static uint8_t ring_next(uint8_t index, uint8_t size)
{
    index++;
    return index >= size ? 0U : index;
}

static void service_tx_fifo(void)
{
    while (s_tx_tail != s_tx_head &&
        !DL_UART_Main_isTXFIFOFull(BLUETOOTH_UART_INST)) {
        DL_UART_Main_transmitData(
            BLUETOOTH_UART_INST, s_tx_buffer[s_tx_tail]);
        s_tx_tail = ring_next(s_tx_tail, TX_BUFFER_SIZE);
    }

    if (s_tx_tail == s_tx_head) {
        DL_UART_Main_disableInterrupt(
            BLUETOOTH_UART_INST, DL_UART_MAIN_INTERRUPT_TX);
    } else {
        DL_UART_Main_enableInterrupt(
            BLUETOOTH_UART_INST, DL_UART_MAIN_INTERRUPT_TX);
    }
}

static void uart_send(const char *text)
{
    uint32_t interrupt_state = __get_PRIMASK();
    __disable_irq();

    while (*text != '\0') {
        uint8_t next = ring_next(s_tx_head, TX_BUFFER_SIZE);
        if (next == s_tx_tail) {
            break;
        }
        s_tx_buffer[s_tx_head] = (uint8_t) *text;
        s_tx_head = next;
        text++;
    }
    service_tx_fifo();

    if (interrupt_state == 0U) {
        __enable_irq();
    }
}

static int rx_pop(uint8_t *value)
{
    if (s_rx_tail == s_rx_head) {
        return 0;
    }
    *value = s_rx_buffer[s_rx_tail];
    s_rx_tail = ring_next(s_rx_tail, RX_BUFFER_SIZE);
    return 1;
}

static char *append_text(char *out, const char *text)
{
    while (*text != '\0') {
        *out++ = *text++;
    }
    return out;
}

static char *append_u32(char *out, uint32_t value)
{
    char digits[10];
    uint8_t count = 0U;

    do {
        digits[count++] = (char) ('0' + value % 10U);
        value /= 10U;
    } while (value != 0U);

    while (count != 0U) {
        *out++ = digits[--count];
    }
    return out;
}

static char *append_i32(char *out, int32_t value)
{
    if (value < 0) {
        *out++ = '-';
        return append_u32(out, (uint32_t) (-(int64_t) value));
    }
    return append_u32(out, (uint32_t) value);
}

static void send_status(void)
{
    char response[96];
    char *out = response;

    out = append_text(out, "S M=");
    out = append_i32(out, g_test.mode);
    out = append_text(out, " ST=");
    out = append_i32(out, g_test.status);
    out = append_text(out, " T=");
    out = append_u32(out, g_test.elapsed_ms);
    out = append_text(out, " L=");
    out = append_i32(out, (int32_t) g_test.left.rpm);
    out = append_text(out, " R=");
    out = append_i32(out, (int32_t) g_test.right.rpm);
    out = append_text(out, " P=");
    out = append_i32(out, g_test.line.position_x1000);
    out = append_text(out, " B=");
    out = append_u32(out, g_test.line.state);
    out = append_text(out, " X=");
    out = append_u32(out, g_test.line.line_lost);
    out = append_text(out, "\r\n");
    *out = '\0';
    uart_send(response);
}

static void process_command(void)
{
    uint32_t now = Platform_Millis();

    if (strcmp(s_command, "PING") == 0) {
        char response[32];
        char *out = append_text(response, "PONG ");
        out = append_u32(out, g_test.build_id);
        out = append_text(out, "\r\n");
        *out = '\0';
        uart_send(response);
    } else if (strcmp(s_command, "STATUS") == 0) {
        send_status();
    } else if (strcmp(s_command, "ARM") == 0) {
        s_bluetooth_armed = 1U;
        s_bluetooth_arm_ms = now;
        uart_send("OK ARM\r\n");
    } else if (strcmp(s_command, "START") == 0) {
        if (s_bluetooth_armed == 0U ||
            (uint32_t) (now - s_bluetooth_arm_ms) >
                BLUETOOTH_ARM_WINDOW_MS) {
            s_bluetooth_armed = 0U;
            uart_send("ERR ARM\r\n");
        } else {
            s_bluetooth_armed = 0U;
            if (TestMode_RequestBluetoothStart()) {
                uart_send("OK START\r\n");
            } else {
                uart_send("ERR START\r\n");
            }
        }
    } else if (strcmp(s_command, "LINE") == 0) {
        if (s_bluetooth_armed == 0U ||
            (uint32_t) (now - s_bluetooth_arm_ms) >
                BLUETOOTH_ARM_WINDOW_MS) {
            s_bluetooth_armed = 0U;
            uart_send("ERR ARM\r\n");
        } else {
            s_bluetooth_armed = 0U;
            if (TestMode_RequestBluetoothLineFollow()) {
                uart_send("OK LINE\r\n");
            } else {
                uart_send("ERR LINE\r\n");
            }
        }
    } else if (strcmp(s_command, "LAP") == 0) {
        if (s_bluetooth_armed == 0U ||
            (uint32_t) (now - s_bluetooth_arm_ms) >
                BLUETOOTH_ARM_WINDOW_MS) {
            s_bluetooth_armed = 0U;
            uart_send("ERR ARM\r\n");
        } else {
            s_bluetooth_armed = 0U;
            if (TestMode_RequestBluetoothLineLap()) {
                uart_send("OK LAP\r\n");
            } else {
                uart_send("ERR LAP\r\n");
            }
        }
    } else if (strcmp(s_command, "KEEP") == 0 ||
        strcmp(s_command, "K") == 0) {
        TestMode_BluetoothKeepalive();
    } else if (strcmp(s_command, "STOP") == 0) {
        s_bluetooth_armed = 0U;
        TestMode_Stop();
        uart_send("OK STOP\r\n");
    } else if (s_command[0] != '+') {
        uart_send("ERR CMD\r\n");
    }
}

static void consume_byte(uint8_t byte)
{
    if (byte == '\r' || byte == '\n') {
        if (s_command_length != 0U) {
            if (s_command_discard == 0U) {
                s_command[s_command_length] = '\0';
                process_command();
            } else {
                uart_send("ERR LONG\r\n");
            }
        }
        s_command_length = 0U;
        s_command_discard = 0U;
        return;
    }

    if (byte >= 'a' && byte <= 'z') {
        byte = (uint8_t) (byte - ('a' - 'A'));
    }
    if (byte < 0x20U || byte > 0x7EU) {
        return;
    }
    if (s_command_discard != 0U) {
        return;
    }
    if (s_command_length >= COMMAND_BUFFER_SIZE - 1U) {
        s_command_discard = 1U;
        return;
    }
    s_command[s_command_length++] = (char) byte;
}

void Bluetooth_Init(void)
{
    s_rx_head = 0U;
    s_rx_tail = 0U;
    s_rx_overflow = 0U;
    s_tx_head = 0U;
    s_tx_tail = 0U;
    s_command_length = 0U;
    s_command_discard = 0U;
    s_bluetooth_armed = 0U;
    s_bluetooth_arm_ms = Platform_Millis();

    DL_UART_Main_enableInterrupt(
        BLUETOOTH_UART_INST, DL_UART_MAIN_INTERRUPT_RX);
    NVIC_ClearPendingIRQ(BLUETOOTH_UART_INST_INT_IRQN);
    NVIC_EnableIRQ(BLUETOOTH_UART_INST_INT_IRQN);
}

void Bluetooth_Tick(void)
{
    if (s_bluetooth_armed != 0U &&
        (uint32_t) (Platform_Millis() - s_bluetooth_arm_ms) >
            BLUETOOTH_ARM_WINDOW_MS) {
        s_bluetooth_armed = 0U;
    }

    if (s_rx_overflow != 0U) {
        uint32_t interrupt_state = __get_PRIMASK();
        __disable_irq();
        s_rx_tail = s_rx_head;
        s_rx_overflow = 0U;
        if (interrupt_state == 0U) {
            __enable_irq();
        }
        s_command_length = 0U;
        s_command_discard = 0U;
        uart_send("ERR RX\r\n");
    }

    uint8_t byte;
    while (rx_pop(&byte)) {
        consume_byte(byte);
    }
}

void BLUETOOTH_UART_INST_IRQHandler(void)
{
    for (;;) {
        uint32_t interrupt =
            DL_UART_Main_getPendingInterrupt(BLUETOOTH_UART_INST);
        if (interrupt == DL_UART_MAIN_IIDX_NO_INTERRUPT) {
            break;
        }

        if (interrupt == DL_UART_MAIN_IIDX_RX) {
            while (!DL_UART_Main_isRXFIFOEmpty(BLUETOOTH_UART_INST)) {
                uint8_t byte = (uint8_t)
                    DL_UART_Main_receiveData(BLUETOOTH_UART_INST);
                uint8_t next = ring_next(s_rx_head, RX_BUFFER_SIZE);
                if (next == s_rx_tail) {
                    s_rx_overflow = 1U;
                } else {
                    s_rx_buffer[s_rx_head] = byte;
                    s_rx_head = next;
                }
            }
        } else if (interrupt == DL_UART_MAIN_IIDX_TX) {
            service_tx_fifo();
        }
    }
}
