#include "line_sensor.h"

#include <stdbool.h>

#include "platform.h"
#include "ti_msp_dl_config.h"

#define LINE_SENSOR_I2C_ADDRESS          0x5DU
#define LINE_SENSOR_STATE_REGISTER       5U
#define LINE_SENSOR_ANALOG_REGISTER      6U
#define LINE_SENSOR_THRESHOLD_REGISTER  22U

#define LINE_SENSOR_TRANSFER_TIMEOUT_MS  3U
#define LINE_SENSOR_RETRY_PERIOD_MS    250U
#define LINE_SENSOR_THRESHOLD_PERIOD_MS 80U
#define LINE_SENSOR_DISCONNECT_ERRORS     3U
#define I2C_ERRATA_DELAY_CYCLES          32U

static LineSensorData s_data;
static uint32_t s_last_probe_ms;
static uint32_t s_last_threshold_ms;
static uint8_t s_analog_channel;
static uint8_t s_threshold_channel;
static uint8_t s_consecutive_errors;

/*
 * S1 is vehicle-left and state bit 0. One adjacent-sensor spacing is 1000,
 * so the center between S4 and S5 is zero and no floating point is needed.
 */
static const int32_t s_position_weight_x1000[LINE_SENSOR_CHANNEL_COUNT] = {
    -3500, -2500, -1500, -500, 500, 1500, 2500, 3500
};

static void recover_controller(void)
{
    DL_I2C_disableController(LINE_SENSOR_I2C_INST);
    DL_I2C_resetControllerTransfer(LINE_SENSOR_I2C_INST);
    DL_I2C_flushControllerTXFIFO(LINE_SENSOR_I2C_INST);
    DL_I2C_flushControllerRXFIFO(LINE_SENSOR_I2C_INST);
    DL_I2C_clearInterruptStatus(LINE_SENSOR_I2C_INST, UINT32_MAX);
    DL_I2C_enableController(LINE_SENSOR_I2C_INST);
}

static int timed_out(uint32_t start_ms)
{
    return (uint32_t) (Platform_Millis() - start_ms) >=
        LINE_SENSOR_TRANSFER_TIMEOUT_MS;
}

static int wait_controller_idle(void)
{
    uint32_t start_ms = Platform_Millis();
    while ((DL_I2C_getControllerStatus(LINE_SENSOR_I2C_INST) &
            DL_I2C_CONTROLLER_STATUS_IDLE) == 0U) {
        if (timed_out(start_ms)) {
            return 0;
        }
    }
    return 1;
}

static int wait_transfer_complete(void)
{
    uint32_t start_ms = Platform_Millis();
    for (;;) {
        uint32_t status =
            DL_I2C_getControllerStatus(LINE_SENSOR_I2C_INST);
        if ((status & DL_I2C_CONTROLLER_STATUS_ERROR) != 0U) {
            return 0;
        }
        if ((status & DL_I2C_CONTROLLER_STATUS_BUSY) == 0U) {
            return 1;
        }
        if (timed_out(start_ms)) {
            return 0;
        }
    }
}

static int write_register_pointer(uint8_t register_address)
{
    uint32_t status =
        DL_I2C_getControllerStatus(LINE_SENSOR_I2C_INST);
    if ((status & DL_I2C_CONTROLLER_STATUS_ERROR) != 0U) {
        recover_controller();
    }
    if (!wait_controller_idle()) {
        recover_controller();
        return 0;
    }

    DL_I2C_flushControllerTXFIFO(LINE_SENSOR_I2C_INST);
    DL_I2C_fillControllerTXFIFO(
        LINE_SENSOR_I2C_INST, &register_address, 1U);
    DL_I2C_startControllerTransfer(LINE_SENSOR_I2C_INST,
        LINE_SENSOR_I2C_ADDRESS, DL_I2C_CONTROLLER_DIRECTION_TX, 1U);

    /* MSPM0 I2C_ERR_13 requires a short delay after starting a transfer. */
    delay_cycles(I2C_ERRATA_DELAY_CYCLES);
    if (!wait_transfer_complete()) {
        recover_controller();
        return 0;
    }
    return 1;
}

static int read_registers(
    uint8_t register_address, uint8_t *buffer, uint16_t length)
{
    if (buffer == 0 || length == 0U ||
        !write_register_pointer(register_address) ||
        !wait_controller_idle()) {
        return 0;
    }

    DL_I2C_flushControllerRXFIFO(LINE_SENSOR_I2C_INST);
    DL_I2C_startControllerTransfer(LINE_SENSOR_I2C_INST,
        LINE_SENSOR_I2C_ADDRESS, DL_I2C_CONTROLLER_DIRECTION_RX, length);
    delay_cycles(I2C_ERRATA_DELAY_CYCLES);

    uint32_t start_ms = Platform_Millis();
    for (uint16_t i = 0U; i < length; i++) {
        while (DL_I2C_isControllerRXFIFOEmpty(LINE_SENSOR_I2C_INST)) {
            uint32_t status =
                DL_I2C_getControllerStatus(LINE_SENSOR_I2C_INST);
            if ((status & DL_I2C_CONTROLLER_STATUS_ERROR) != 0U ||
                timed_out(start_ms)) {
                recover_controller();
                return 0;
            }
        }
        buffer[i] = DL_I2C_receiveControllerData(LINE_SENSOR_I2C_INST);
    }

    if (!wait_transfer_complete()) {
        recover_controller();
        return 0;
    }
    return 1;
}

static void record_error(void)
{
    s_data.error_count++;
    if (s_consecutive_errors < UINT8_MAX) {
        s_consecutive_errors++;
    }
    if (s_consecutive_errors >= LINE_SENSOR_DISCONNECT_ERRORS) {
        s_data.connected = 0U;
    }
}

static int update_state(void)
{
    uint8_t state = 0U;
    if (!read_registers(
            LINE_SENSOR_STATE_REGISTER, &state, sizeof(state))) {
        record_error();
        return 0;
    }

    s_data.state = state;
    int32_t weighted_sum = 0;
    uint8_t active_count = 0U;
    for (uint8_t i = 0U; i < LINE_SENSOR_CHANNEL_COUNT; i++) {
        if ((state & (uint8_t) (1U << i)) != 0U) {
            weighted_sum += s_position_weight_x1000[i];
            active_count++;
        }
    }
    s_data.active_count = active_count;
    s_data.line_lost = active_count == 0U ? 1U : 0U;
    s_data.position_x1000 =
        active_count == 0U ? 0 : weighted_sum / (int32_t) active_count;
    s_data.connected = 1U;
    s_data.sample_sequence++;
    s_consecutive_errors = 0U;
    return 1;
}

static void update_analog_channel(void)
{
    uint8_t raw[2];
    uint8_t channel = s_analog_channel;
    uint8_t register_address =
        (uint8_t) (LINE_SENSOR_ANALOG_REGISTER + channel * 2U);

    if (read_registers(register_address, raw, sizeof(raw))) {
        s_data.analog[channel] =
            (uint16_t) raw[0] | ((uint16_t) raw[1] << 8);
        s_analog_channel =
            (uint8_t) ((channel + 1U) % LINE_SENSOR_CHANNEL_COUNT);
    } else {
        record_error();
    }
}

static void update_threshold_channel(uint32_t now)
{
    if ((uint32_t) (now - s_last_threshold_ms) <
        LINE_SENSOR_THRESHOLD_PERIOD_MS) {
        return;
    }
    s_last_threshold_ms = now;

    uint8_t raw[2];
    uint8_t channel = s_threshold_channel;
    uint8_t register_address =
        (uint8_t) (LINE_SENSOR_THRESHOLD_REGISTER + channel * 2U);

    if (read_registers(register_address, raw, sizeof(raw))) {
        s_data.threshold[channel] =
            (uint16_t) raw[0] | ((uint16_t) raw[1] << 8);
        s_threshold_channel =
            (uint8_t) ((channel + 1U) % LINE_SENSOR_CHANNEL_COUNT);
    } else {
        record_error();
    }
}

void LineSensor_Init(void)
{
    s_data.connected = 0U;
    s_data.sample_sequence = 0U;
    s_data.error_count = 0U;
    s_data.state = 0U;
    s_data.active_count = 0U;
    s_data.line_lost = 1U;
    s_data.position_x1000 = 0;
    for (uint32_t i = 0U; i < LINE_SENSOR_CHANNEL_COUNT; i++) {
        s_data.analog[i] = 0U;
        s_data.threshold[i] = 0U;
    }

    uint32_t now = Platform_Millis();
    /*
     * The sensor MCU can take longer to boot than the MSPM0. Delay the first
     * address probe so a normal power-up NACK does not become the first bus
     * transaction.
     */
    s_last_probe_ms = now;
    s_last_threshold_ms = now;
    s_analog_channel = 0U;
    s_threshold_channel = 0U;
    s_consecutive_errors = 0U;
}

void LineSensor_Tick(void)
{
    uint32_t now = Platform_Millis();
    if (s_data.connected == 0U &&
        (uint32_t) (now - s_last_probe_ms) <
            LINE_SENSOR_RETRY_PERIOD_MS) {
        return;
    }
    s_last_probe_ms = now;

    if (!update_state()) {
        return;
    }
    update_analog_channel();
    if (s_data.connected != 0U) {
        update_threshold_channel(now);
    }
}

const LineSensorData *LineSensor_GetData(void)
{
    return &s_data;
}
