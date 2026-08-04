#include "oled_display.h"

#include <stddef.h>

#include "platform.h"
#include "ti_msp_dl_config.h"

#define OLED_ADDRESS_PRIMARY               0x3CU
#define OLED_ADDRESS_SECONDARY             0x3DU
#define OLED_TRANSFER_TIMEOUT_MS              5U
#define OLED_BOOT_TRANSFER_TIMEOUT_MS        20U
#define OLED_BOOT_DELAY_MS                  1000U
#define OLED_RECONNECT_PERIOD_MS           1000U
#define OLED_I2C_PACKET_BYTES                 8U
#define OLED_GLYPH_COLUMNS                    6U
#define OLED_VALUE_PAGE                       2U
#define OLED_VALUE_COLUMN                    78U
#define OLED_VALUE_GLYPHS                     4U
#define OLED_POWER_PAGE                       6U
#define OLED_POWER_LABEL_COLUMN              48U
#define OLED_POWER_VALUE_COLUMN              72U
#define OLED_MAX_CONSECUTIVE_ERRORS           3U
#define OLED_I2C_ERRATA_DELAY_CYCLES         32U

static uint8_t s_connected;
static uint8_t s_address;
static uint8_t s_render_pending;
static uint8_t s_power_render_pending;
static uint8_t s_initial_connect_pending;
static uint8_t s_consecutive_errors;
static uint32_t s_error_count;
static uint32_t s_last_reconnect_ms;
static uint32_t s_requested_seconds;
static uint32_t s_displayed_seconds;
static uint8_t s_requested_power;
static uint8_t s_displayed_power;
static uint32_t s_render_count;
static char s_value_text[OLED_VALUE_GLYPHS];

static void recover_controller(void)
{
    DL_I2C_disableController(LINE_SENSOR_I2C_INST);
    DL_I2C_resetControllerTransfer(LINE_SENSOR_I2C_INST);
    DL_I2C_flushControllerTXFIFO(LINE_SENSOR_I2C_INST);
    DL_I2C_flushControllerRXFIFO(LINE_SENSOR_I2C_INST);
    DL_I2C_clearInterruptStatus(LINE_SENSOR_I2C_INST, UINT32_MAX);
    DL_I2C_enableController(LINE_SENSOR_I2C_INST);
}

static int timed_out(uint32_t start_ms, uint32_t timeout_ms)
{
    return (uint32_t) (Platform_Millis() - start_ms) >= timeout_ms;
}

static int wait_controller_idle(uint32_t timeout_ms)
{
    uint32_t start_ms = Platform_Millis();
    while ((DL_I2C_getControllerStatus(LINE_SENSOR_I2C_INST) &
            DL_I2C_CONTROLLER_STATUS_IDLE) == 0U) {
        if (timed_out(start_ms, timeout_ms)) {
            return 0;
        }
    }
    return 1;
}

static int write_packet(
    uint8_t address, const uint8_t *packet, uint16_t length,
    uint32_t timeout_ms)
{
    if (packet == NULL || length == 0U ||
        length > OLED_I2C_PACKET_BYTES) {
        return 0;
    }

    uint32_t status =
        DL_I2C_getControllerStatus(LINE_SENSOR_I2C_INST);
    if ((status & DL_I2C_CONTROLLER_STATUS_ERROR) != 0U) {
        recover_controller();
    }
    if (!wait_controller_idle(timeout_ms)) {
        recover_controller();
        return 0;
    }

    DL_I2C_flushControllerTXFIFO(LINE_SENSOR_I2C_INST);
    DL_I2C_fillControllerTXFIFO(
        LINE_SENSOR_I2C_INST, (uint8_t *) packet, length);
    DL_I2C_startControllerTransfer(LINE_SENSOR_I2C_INST,
        address, DL_I2C_CONTROLLER_DIRECTION_TX, length);
    delay_cycles(OLED_I2C_ERRATA_DELAY_CYCLES);

    uint32_t start_ms = Platform_Millis();
    for (;;) {
        status = DL_I2C_getControllerStatus(LINE_SENSOR_I2C_INST);
        if ((status & DL_I2C_CONTROLLER_STATUS_ERROR) != 0U) {
            recover_controller();
            return 0;
        }
        if ((status & DL_I2C_CONTROLLER_STATUS_BUSY) == 0U) {
            return 1;
        }
        if (timed_out(start_ms, timeout_ms)) {
            recover_controller();
            return 0;
        }
    }
}

static int write_commands(
    uint8_t address, const uint8_t *commands, uint16_t count,
    uint32_t timeout_ms)
{
    while (count != 0U) {
        uint8_t packet[OLED_I2C_PACKET_BYTES];
        uint16_t chunk = count;
        if (chunk > OLED_I2C_PACKET_BYTES - 1U) {
            chunk = OLED_I2C_PACKET_BYTES - 1U;
        }
        packet[0] = 0x00U;
        for (uint16_t i = 0U; i < chunk; i++) {
            packet[i + 1U] = commands[i];
        }
        if (!write_packet(address, packet, chunk + 1U, timeout_ms)) {
            return 0;
        }
        commands += chunk;
        count -= chunk;
    }
    return 1;
}

static int set_page_column(uint8_t page, uint8_t column)
{
    uint8_t commands[3] = {
        (uint8_t) (0xB0U | (page & 0x07U)),
        (uint8_t) (column & 0x0FU),
        (uint8_t) (0x10U | ((column >> 4) & 0x0FU))
    };
    return write_commands(
        s_address, commands, sizeof(commands),
        OLED_TRANSFER_TIMEOUT_MS);
}

static int write_data(const uint8_t *data, uint16_t length)
{
    while (length != 0U) {
        uint8_t packet[OLED_I2C_PACKET_BYTES];
        uint16_t chunk = length;
        if (chunk > OLED_I2C_PACKET_BYTES - 1U) {
            chunk = OLED_I2C_PACKET_BYTES - 1U;
        }
        packet[0] = 0x40U;
        for (uint16_t i = 0U; i < chunk; i++) {
            packet[i + 1U] = data[i];
        }
        if (!write_packet(
                s_address, packet, chunk + 1U,
                OLED_TRANSFER_TIMEOUT_MS)) {
            return 0;
        }
        data += chunk;
        length -= chunk;
    }
    return 1;
}

static void glyph_for(char character, uint8_t glyph[OLED_GLYPH_COLUMNS])
{
    static const uint8_t digits[10][5] = {
        {0x3EU, 0x51U, 0x49U, 0x45U, 0x3EU},
        {0x00U, 0x42U, 0x7FU, 0x40U, 0x00U},
        {0x42U, 0x61U, 0x51U, 0x49U, 0x46U},
        {0x21U, 0x41U, 0x45U, 0x4BU, 0x31U},
        {0x18U, 0x14U, 0x12U, 0x7FU, 0x10U},
        {0x27U, 0x45U, 0x45U, 0x45U, 0x39U},
        {0x3CU, 0x4AU, 0x49U, 0x49U, 0x30U},
        {0x01U, 0x71U, 0x09U, 0x05U, 0x03U},
        {0x36U, 0x49U, 0x49U, 0x49U, 0x36U},
        {0x06U, 0x49U, 0x49U, 0x29U, 0x1EU}
    };
    const uint8_t *source = NULL;
    static const uint8_t glyph_t[5] =
        {0x01U, 0x01U, 0x7FU, 0x01U, 0x01U};
    static const uint8_t glyph_i[5] =
        {0x00U, 0x41U, 0x7FU, 0x41U, 0x00U};
    static const uint8_t glyph_m[5] =
        {0x7FU, 0x02U, 0x0CU, 0x02U, 0x7FU};
    static const uint8_t glyph_e[5] =
        {0x7FU, 0x49U, 0x49U, 0x49U, 0x41U};
    static const uint8_t glyph_s[5] =
        {0x46U, 0x49U, 0x49U, 0x49U, 0x31U};
    static const uint8_t glyph_p[5] =
        {0x7FU, 0x09U, 0x09U, 0x09U, 0x06U};
    static const uint8_t glyph_w[5] =
        {0x3FU, 0x40U, 0x38U, 0x40U, 0x3FU};
    static const uint8_t glyph_r[5] =
        {0x7FU, 0x09U, 0x19U, 0x29U, 0x46U};

    if (character >= '0' && character <= '9') {
        source = digits[(uint8_t) (character - '0')];
    } else if (character == 'T') {
        source = glyph_t;
    } else if (character == 'I') {
        source = glyph_i;
    } else if (character == 'M') {
        source = glyph_m;
    } else if (character == 'E') {
        source = glyph_e;
    } else if (character == 'S') {
        source = glyph_s;
    } else if (character == 'P') {
        source = glyph_p;
    } else if (character == 'W') {
        source = glyph_w;
    } else if (character == 'R') {
        source = glyph_r;
    }

    for (uint8_t i = 0U; i < 5U; i++) {
        glyph[i] = source == NULL ? 0U : source[i];
    }
    glyph[5] = 0U;
}

static int write_character(uint8_t page, uint8_t column, char character)
{
    uint8_t glyph[OLED_GLYPH_COLUMNS];
    glyph_for(character, glyph);
    return set_page_column(page, column) &&
        write_data(glyph, sizeof(glyph));
}

static int clear_display(void)
{
    uint8_t zeros[7] = {0U, 0U, 0U, 0U, 0U, 0U, 0U};
    for (uint8_t page = 0U; page < 8U; page++) {
        if (!set_page_column(page, 0U)) {
            return 0;
        }
        uint16_t remaining = 128U;
        while (remaining != 0U) {
            uint16_t chunk = remaining > sizeof(zeros) ?
                sizeof(zeros) : remaining;
            if (!write_data(zeros, chunk)) {
                return 0;
            }
            remaining -= chunk;
        }
    }
    return 1;
}

static int initialize_at_address(uint8_t address)
{
    static const uint8_t init_commands[] = {
        0xAEU,       /* display off */
        0xD5U, 0x80U,
        0xA8U, 0x3FU,
        0xD3U, 0x00U,
        0x40U,
        0x8DU, 0x14U,
        0x20U, 0x02U, /* page addressing mode */
        0xA1U,
        0xC8U,
        0xDAU, 0x12U,
        0x81U, 0x7FU,
        0xD9U, 0xF1U,
        0xDBU, 0x40U,
        0xA4U,
        0xA6U,
        0x2EU,
        0xAFU
    };
    if (!write_commands(
            address, init_commands, sizeof(init_commands),
            OLED_BOOT_TRANSFER_TIMEOUT_MS)) {
        return 0;
    }
    s_address = address;
    return clear_display();
}

static void record_runtime_error(void)
{
    s_error_count++;
    if (s_consecutive_errors < UINT8_MAX) {
        s_consecutive_errors++;
    }
    if (s_consecutive_errors >= OLED_MAX_CONSECUTIVE_ERRORS) {
        s_connected = 0U;
        s_last_reconnect_ms = Platform_Millis();
        s_render_pending = 0U;
        s_power_render_pending = 0U;
    }
}

static void format_seconds(uint32_t seconds)
{
    uint32_t display_seconds = seconds % 1000U;
    s_value_text[0] =
        display_seconds >= 100U ?
        (char) ('0' + display_seconds / 100U) : ' ';
    s_value_text[1] =
        display_seconds >= 10U ?
        (char) ('0' + (display_seconds / 10U) % 10U) : ' ';
    s_value_text[2] = (char) ('0' + display_seconds % 10U);
    s_value_text[3] = 'S';
    s_render_pending = 1U;
}

static int render_time_value(void)
{
    uint8_t value_pixels[OLED_VALUE_GLYPHS * OLED_GLYPH_COLUMNS];
    for (uint8_t index = 0U; index < OLED_VALUE_GLYPHS; index++) {
        uint8_t glyph[OLED_GLYPH_COLUMNS];
        glyph_for(s_value_text[index], glyph);
        for (uint8_t column = 0U; column < OLED_GLYPH_COLUMNS; column++) {
            value_pixels[index * OLED_GLYPH_COLUMNS + column] =
                glyph[column];
        }
    }
    /*
     * Set the cursor once and stream all four glyphs in display order.  The
     * I2C FIFO still splits the bytes into bounded packets, but the visible
     * time field no longer jumps between four separately repositioned writes.
     */
    return set_page_column(OLED_VALUE_PAGE, OLED_VALUE_COLUMN) &&
        write_data(value_pixels, sizeof(value_pixels));
}

static int connect_and_draw_static_ui(void)
{
    s_address = 0U;
    if (!initialize_at_address(OLED_ADDRESS_PRIMARY) &&
        !initialize_at_address(OLED_ADDRESS_SECONDARY)) {
        return 0;
    }

    s_connected = 1U;
    if (!write_character(2U, 50U, 'T') ||
        !write_character(2U, 56U, 'I') ||
        !write_character(2U, 62U, 'M') ||
        !write_character(2U, 68U, 'E') ||
        !write_character(OLED_POWER_PAGE, OLED_POWER_LABEL_COLUMN, 'P') ||
        !write_character(
            OLED_POWER_PAGE,
            OLED_POWER_LABEL_COLUMN + OLED_GLYPH_COLUMNS, 'W') ||
        !write_character(
            OLED_POWER_PAGE,
            OLED_POWER_LABEL_COLUMN + 2U * OLED_GLYPH_COLUMNS, 'R')) {
        s_connected = 0U;
        s_last_reconnect_ms = Platform_Millis();
        return 0;
    }

    s_consecutive_errors = 0U;
    s_displayed_seconds = UINT32_MAX;
    s_displayed_power = UINT8_MAX;
    format_seconds(s_requested_seconds);
    s_power_render_pending = 1U;
    return 1;
}

void OledDisplay_Init(void)
{
    s_connected = 0U;
    s_address = 0U;
    s_render_pending = 0U;
    s_power_render_pending = 0U;
    s_initial_connect_pending = 1U;
    s_consecutive_errors = 0U;
    s_error_count = 0U;
    s_last_reconnect_ms = Platform_Millis();
    s_requested_seconds = 0U;
    s_displayed_seconds = UINT32_MAX;
    s_requested_power = 0U;
    s_displayed_power = UINT8_MAX;
    s_render_count = 0U;

    /*
     * Do not talk to the externally powered OLED during the regulator's
     * startup ramp. OledDisplay_Tick() performs the first attempt after the
     * non-blocking boot interval, while UART, buttons and motor safety start
     * immediately. A failed first attempt falls into the normal retry path.
     */
}

void OledDisplay_Tick(
    uint32_t elapsed_seconds, uint8_t power_enabled)
{
    if (elapsed_seconds != s_requested_seconds ||
        s_displayed_seconds == UINT32_MAX) {
        s_requested_seconds = elapsed_seconds;
        format_seconds(elapsed_seconds);
    }
    power_enabled = power_enabled != 0U ? 1U : 0U;
    if (power_enabled != s_requested_power ||
        s_displayed_power == UINT8_MAX) {
        s_requested_power = power_enabled;
        s_power_render_pending = 1U;
    }

    if (s_connected == 0U) {
        uint32_t now = Platform_Millis();
        uint32_t wait_ms = s_initial_connect_pending != 0U ?
            OLED_BOOT_DELAY_MS : OLED_RECONNECT_PERIOD_MS;
        if ((uint32_t)(now - s_last_reconnect_ms) >= wait_ms) {
            s_initial_connect_pending = 0U;
            s_last_reconnect_ms = now;
            recover_controller();
            (void) connect_and_draw_static_ui();
        }
        return;
    }
    if (s_power_render_pending != 0U) {
        if (!write_character(
                OLED_POWER_PAGE, OLED_POWER_VALUE_COLUMN,
                s_requested_power != 0U ? '1' : '0')) {
            record_runtime_error();
            return;
        }
        s_consecutive_errors = 0U;
        s_power_render_pending = 0U;
        s_displayed_power = s_requested_power;
    }
    if (s_render_pending == 0U) {
        return;
    }
    if (!render_time_value()) {
        record_runtime_error();
        return;
    }
    s_consecutive_errors = 0U;
    s_render_pending = 0U;
    s_displayed_seconds = s_requested_seconds;
    s_render_count++;
}

uint8_t OledDisplay_IsConnected(void)
{
    return s_connected;
}

uint8_t OledDisplay_GetAddress(void)
{
    return s_address;
}

uint32_t OledDisplay_GetErrorCount(void)
{
    return s_error_count;
}

uint32_t OledDisplay_GetRequestedSeconds(void)
{
    return s_requested_seconds;
}

uint32_t OledDisplay_GetDisplayedSeconds(void)
{
    return s_displayed_seconds;
}

uint32_t OledDisplay_GetRenderCount(void)
{
    return s_render_count;
}
