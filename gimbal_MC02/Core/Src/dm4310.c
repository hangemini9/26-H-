#include "dm4310.h"
#include "gimbal_config.h"
#include <string.h>

static FDCAN_HandleTypeDef *s_can;
static Dm4310Status s_status;

static float clamp_float(float value, float minimum, float maximum)
{
    if (value < minimum)
    {
        return minimum;
    }
    if (value > maximum)
    {
        return maximum;
    }
    return value;
}

static float uint_to_float(uint32_t value,
                           float minimum,
                           float maximum,
                           uint32_t bits)
{
    const uint32_t full_scale = (1UL << bits) - 1UL;
    return ((float)value * (maximum - minimum) / (float)full_scale) + minimum;
}

static HAL_StatusTypeDef send_frame(uint32_t identifier,
                                    const uint8_t *data,
                                    uint32_t dlc)
{
    FDCAN_TxHeaderTypeDef header;
    HAL_StatusTypeDef result;
    uint8_t payload[8] = {0U};

    if (s_can == NULL)
    {
        return HAL_ERROR;
    }

    if ((data != NULL) && (dlc == FDCAN_DLC_BYTES_8))
    {
        memcpy(payload, data, sizeof(payload));
    }

    memset(&header, 0, sizeof(header));
    header.Identifier = identifier;
    header.IdType = FDCAN_STANDARD_ID;
    header.TxFrameType = FDCAN_DATA_FRAME;
    header.DataLength = dlc;
    header.ErrorStateIndicator = FDCAN_ESI_ACTIVE;
    header.BitRateSwitch = FDCAN_BRS_OFF;
    header.FDFormat = FDCAN_CLASSIC_CAN;
    header.TxEventFifoControl = FDCAN_NO_TX_EVENTS;
    header.MessageMarker = 0U;

    result = HAL_FDCAN_AddMessageToTxFifoQ(s_can, &header, payload);
    if (result == HAL_OK)
    {
        s_status.tx_count++;
    }
    else
    {
        s_status.tx_error_count++;
        s_status.hal_error = HAL_FDCAN_GetError(s_can);
    }
    return result;
}

void dm4310_init(FDCAN_HandleTypeDef *hfdcan, uint32_t now_ms)
{
    FDCAN_FilterTypeDef filter;
    HAL_StatusTypeDef result;

    memset(&s_status, 0, sizeof(s_status));
    s_can = hfdcan;
    s_status.last_rx_ms = now_ms;

    memset(&filter, 0, sizeof(filter));
    filter.IdType = FDCAN_STANDARD_ID;
    filter.FilterIndex = 0U;
    filter.FilterType = FDCAN_FILTER_MASK;
    filter.FilterConfig = FDCAN_FILTER_TO_RXFIFO0;
    filter.FilterID1 = DM4310_MASTER_CAN_ID;
    filter.FilterID2 = 0x7FFU;

    result = HAL_FDCAN_ConfigFilter(s_can, &filter);
    if (result == HAL_OK)
    {
        result = HAL_FDCAN_ConfigGlobalFilter(s_can,
                                              FDCAN_REJECT,
                                              FDCAN_REJECT,
                                              FDCAN_REJECT_REMOTE,
                                              FDCAN_REJECT_REMOTE);
    }
    if (result == HAL_OK)
    {
        result = HAL_FDCAN_Start(s_can);
    }
    if (result != HAL_OK)
    {
        s_status.hal_error = HAL_FDCAN_GetError(s_can);
    }
    else
    {
        s_status.initialized = 1U;
    }
}

void dm4310_poll(uint32_t now_ms)
{
    FDCAN_RxHeaderTypeDef header;
    uint8_t data[8];

    if (s_can == NULL)
    {
        return;
    }

    while (HAL_FDCAN_GetRxFifoFillLevel(s_can, FDCAN_RX_FIFO0) > 0U)
    {
        memset(&header, 0, sizeof(header));
        memset(data, 0, sizeof(data));

        if (HAL_FDCAN_GetRxMessage(s_can,
                                   FDCAN_RX_FIFO0,
                                   &header,
                                   data) != HAL_OK)
        {
            s_status.hal_error = HAL_FDCAN_GetError(s_can);
            break;
        }

        if ((header.IdType != FDCAN_STANDARD_ID) ||
            (header.Identifier != DM4310_MASTER_CAN_ID) ||
            (header.DataLength != FDCAN_DLC_BYTES_8))
        {
            s_status.rx_invalid_count++;
            continue;
        }

        s_status.motor_id = data[0] & 0x0FU;
        s_status.error = (data[0] >> 4) & 0x0FU;
        if (s_status.motor_id != (DM4310_MOTOR_CAN_ID & 0x0FU))
        {
            s_status.rx_invalid_count++;
            continue;
        }

        s_status.position_rad =
            uint_to_float(((uint32_t)data[1] << 8) | data[2],
                          -DM4310_PMAX_RAD,
                          DM4310_PMAX_RAD,
                          16U);
        s_status.velocity_rad_s =
            uint_to_float(((uint32_t)data[3] << 4) | (data[4] >> 4),
                          -DM4310_VMAX_RAD_S,
                          DM4310_VMAX_RAD_S,
                          12U);
        s_status.torque_nm =
            uint_to_float((((uint32_t)data[4] & 0x0FU) << 8) | data[5],
                          -DM4310_TMAX_NM,
                          DM4310_TMAX_NM,
                          12U);
        s_status.mos_temperature_c = data[6];
        s_status.rotor_temperature_c = data[7];
        s_status.last_rx_ms = now_ms;
        s_status.rx_count++;
        s_status.online = 1U;
    }

    if ((s_status.online != 0U) &&
        ((uint32_t)(now_ms - s_status.last_rx_ms) >
         GIMBAL_FEEDBACK_TIMEOUT_MS))
    {
        s_status.online = 0U;
    }
}

HAL_StatusTypeDef dm4310_query(void)
{
    /*
     * Keep OBSERVE torque-disabled and use the selected mode's real control
     * identifier. Bench evidence showed that startup disable frames on 0x101
     * produced feedback, whereas zero-filled 0x001 frames did not produce
     * continuous feedback and allowed the motor's communication watchdog to
     * report error 0xD. Repeating disable is safe in OBSERVE and also asks
     * the drive for its current feedback without ever enabling the MOSFETs.
     */
    const uint8_t data[8] =
        {0xFFU, 0xFFU, 0xFFU, 0xFFU, 0xFFU, 0xFFU, 0xFFU, 0xFDU};
    return send_frame(DM4310_POSITION_SPEED_TX_ID,
                      data,
                      FDCAN_DLC_BYTES_8);
}

HAL_StatusTypeDef dm4310_enable(void)
{
    const uint8_t data[8] =
        {0xFFU, 0xFFU, 0xFFU, 0xFFU, 0xFFU, 0xFFU, 0xFFU, 0xFCU};
    return send_frame(DM4310_POSITION_SPEED_TX_ID,
                      data,
                      FDCAN_DLC_BYTES_8);
}

HAL_StatusTypeDef dm4310_disable(void)
{
    const uint8_t data[8] =
        {0xFFU, 0xFFU, 0xFFU, 0xFFU, 0xFFU, 0xFFU, 0xFFU, 0xFDU};
    return send_frame(DM4310_POSITION_SPEED_TX_ID,
                      data,
                      FDCAN_DLC_BYTES_8);
}

HAL_StatusTypeDef dm4310_command_position_speed(float position_rad,
                                                float velocity_rad_s)
{
    uint8_t data[8];

    position_rad = clamp_float(position_rad,
                               -DM4310_PMAX_RAD,
                               DM4310_PMAX_RAD);
    velocity_rad_s = clamp_float(velocity_rad_s, 0.0f, DM4310_VMAX_RAD_S);

    /*
     * DM-J4310 position-speed mode uses two little-endian IEEE-754 floats:
     * p_des in D0..D3 and the maximum absolute speed v_des in D4..D7.
     */
    memcpy(&data[0], &position_rad, sizeof(position_rad));
    memcpy(&data[4], &velocity_rad_s, sizeof(velocity_rad_s));
    return send_frame(DM4310_POSITION_SPEED_TX_ID,
                      data,
                      FDCAN_DLC_BYTES_8);
}

const Dm4310Status *dm4310_status(void)
{
    return &s_status;
}
