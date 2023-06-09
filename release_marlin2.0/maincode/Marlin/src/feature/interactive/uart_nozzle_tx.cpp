#include "../../inc/ANKER_Config.h"

#if ADAPT_DETACHED_NOZZLE

#include "uart_nozzle_tx.h"

#include "../../inc/MarlinConfig.h"
#include "../../module/temperature.h"
#include "clock.h"

extern "C"
{

#include "uart_nozzle_rx.h"

#include <stdint.h>

#include "gcp.h"
#include "gcp_parser.h"
#include "protocol.h"
#include "soft_timer.h"

    void uart_nozzle_tx_probe_val(uint16_t val)
    {
        uint8_t content[8];
        uint8_t buf[64];
        gcp_msg_t *gcp_msg = (gcp_msg_t *)&buf[0];
        uint16_t len;

        content[0] = val & 0xFF;
        content[1] = val >> 8;
        len = gcp_msg_pack(gcp_msg,
                           GCP_TYPE_SET,
                           GCP_MODULE_01_MARLIN,
                           GCP_MODULE_02_NOZZLE,
                           GCP_CMD_21_PROBE_SET,
                           content,
                           2);

        MYSERIAL3.send((uint8_t *)gcp_msg, len);
    }

    void uart_nozzle_tx_hwsw_ver_get(void)
    {
        uint8_t buf[64];
        gcp_msg_t *gcp_msg = (gcp_msg_t *)&buf[0];
        uint16_t len;

        len = gcp_msg_pack(gcp_msg,
                           GCP_TYPE_CMD,
                           GCP_MODULE_01_MARLIN,
                           GCP_MODULE_02_NOZZLE,
                           GCP_CMD_24_HWSW_VER,
                           0,
                           0);

        MYSERIAL3.send((uint8_t *)gcp_msg, len);
    }

    void uart_nozzle_tx_single_data(uint8_t cmd, uint8_t dat)
    {
        uint8_t buf[64];
        gcp_msg_t *gcp_msg = (gcp_msg_t *)&buf[0];
        uint16_t len;
        uint8_t context = dat;

        len = gcp_msg_pack(gcp_msg,
                           GCP_TYPE_CMD,
                           GCP_MODULE_01_MARLIN,
                           GCP_MODULE_02_NOZZLE,
                           cmd,
                           &context,
                           1);

        MYSERIAL3.send((uint8_t *)gcp_msg, len);
    }

    void uart_nozzle_tx_multi_data(uint8_t cmd, uint8_t *buf, uint16_t size)
    {
        uint8_t pbuf[64];
        gcp_msg_t *gcp_msg = (gcp_msg_t *)&pbuf[0];
        uint16_t len;

        len = gcp_msg_pack(gcp_msg,
                           GCP_TYPE_CMD,
                           GCP_MODULE_01_MARLIN,
                           GCP_MODULE_02_NOZZLE,
                           cmd,
                           buf,
                           size);

        MYSERIAL3.send((uint8_t *)gcp_msg, len);
    }

    void uart_nozzle_tx_show_adc_value_on()
    {
        uint8_t buf[64];
        gcp_msg_t *gcp_msg = (gcp_msg_t *)&buf[0];
        uint16_t len;

        len = gcp_msg_pack(gcp_msg,
                           GCP_TYPE_SET,
                           GCP_MODULE_01_MARLIN,
                           GCP_MODULE_02_NOZZLE,
                           GCP_CMD_41_SHOW_ADC_VALUE_ON,
                           0,
                           0);

        MYSERIAL3.send((uint8_t *)gcp_msg, len);
    }

    void uart_nozzle_tx_show_adc_value_off()
    {
        uint8_t buf[64];
        gcp_msg_t *gcp_msg = (gcp_msg_t *)&buf[0];
        uint16_t len;

        len = gcp_msg_pack(gcp_msg,
                           GCP_TYPE_SET,
                           GCP_MODULE_01_MARLIN,
                           GCP_MODULE_02_NOZZLE,
                           GCP_CMD_42_SHOW_ADC_VALUE_OFF,
                           0,
                           0);

        MYSERIAL3.send((uint8_t *)gcp_msg, len);
    }

    void uart_nozzle_tx_pid_autotune(uint16_t temp, uint16_t ncycles)
    {
        uint8_t content[8];
        uint8_t buf[64];
        gcp_msg_t *gcp_msg = (gcp_msg_t *)&buf[0];
        uint16_t len;

        content[0] = temp & 0xFF;
        content[1] = temp >> 8;
        content[2] = ncycles & 0xFF;
        content[3] = ncycles >> 8;

        len = gcp_msg_pack(gcp_msg,
                           GCP_TYPE_SET,
                           GCP_MODULE_01_MARLIN,
                           GCP_MODULE_02_NOZZLE,
                           GCP_CMD_43_PID_AUTO_TURN,
                           content,
                           4);

        MYSERIAL3.send((uint8_t *)gcp_msg, len);
    }

    // Get strain gauge readings
    void uart_nozzle_tx_probe_val_get()
    {
        uint8_t buf[64];
        gcp_msg_t *gcp_msg = (gcp_msg_t *)&buf[0];
        uint16_t len;

        len = gcp_msg_pack(gcp_msg,
                           GCP_TYPE_SET,
                           GCP_MODULE_01_MARLIN,
                           GCP_MODULE_02_NOZZLE,
                           GCP_CMD_40_GET_PROBE_VALUE,
                           0,
                           0);

        MYSERIAL3.send((uint8_t *)gcp_msg, len);
    }

    // set probe leveling value
    void uart_nozzle_tx_probe_leveling_val_set(int16_t val)
    {
        uint8_t buf[64];
        gcp_msg_t *gcp_msg = (gcp_msg_t *)&buf[0];
        uint16_t len;

        len = gcp_msg_pack(gcp_msg,
                           GCP_TYPE_SET,
                           GCP_MODULE_01_MARLIN,
                           GCP_MODULE_02_NOZZLE,
                           GCP_CMD_33_LEVELING_VAL_WRITE,
                           (uint8_t *)&val,
                           2);

        MYSERIAL3.send((uint8_t *)gcp_msg, len);
    }

    // get probe leveling value
    void uart_nozzle_tx_probe_leveling_val_get(void)
    {
        uint8_t buf[64];
        gcp_msg_t *gcp_msg = (gcp_msg_t *)&buf[0];
        uint16_t len;

        len = gcp_msg_pack(gcp_msg,
                           GCP_TYPE_SET,
                           GCP_MODULE_01_MARLIN,
                           GCP_MODULE_02_NOZZLE,
                           GCP_CMD_34_LEVELING_VAL_READ,
                           0,
                           0);

        MYSERIAL3.send((uint8_t *)gcp_msg, len);
    }

    void uart_nozzle_tx_auto_offset_start(void)
    {
        uint8_t buf[64];
        gcp_msg_t *gcp_msg = (gcp_msg_t *)&buf[0];
        uint16_t len;

        len = gcp_msg_pack(gcp_msg,
                           GCP_TYPE_CMD,
                           GCP_MODULE_01_MARLIN,
                           GCP_MODULE_02_NOZZLE,
                           GCP_CMD_43_AUTO_OFFSET_START,
                           0,
                           0);

        MYSERIAL3.send((uint8_t *)gcp_msg, len);
    }
}

#endif
