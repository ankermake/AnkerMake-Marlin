#include "../../inc/ANKER_Config.h"

#if ADAPT_DETACHED_NOZZLE

#include "../../gcode/gcode.h"
#include "../../inc/MarlinConfig.h"
#include "../../module/settings.h"
#include "../anker/anker_z_offset.h"
#include "M3011_3100.h"
#include <string.h>

#include "oci.h"
#include "protocol.h"
#include "uart_nozzle_rx.h"
#include "uart_nozzle_tx.h"
#include <stdint.h>

// hw/sw version get, M3011 S1/S0
void GcodeSuite::M3011(void)
{
    uint8_t n = 0xFF;
    uint8_t data[40];
    uint32_t addr = 0x0800D000;

    if (parser.seenval('S'))
        n = parser.value_ushort();

    switch (n)
    {
    case 0: // get marlin boot version
        if (strstr((char *)addr, "ANKER"))
        {
            memcpy(data, (uint8_t *)addr, 23);
        }
        else
        {
            memcpy(data, (uint8_t *)"ANKER_V8111_BOOT_V0.0.0", 23);
        }
        data[23] = '\n';
        data[24] = '\0';
        data[25] = '\0';

        MYSERIAL2.printLine("%s",data);
        break;

    case 1: // get nozzle hw&sw verion
        if (nozzle_board_type == NOZZLE_TYPE_OLD)
        {
            MYSERIAL2.printLine("ANKER_V8111_HW_V0.0.0_SW_V0.0.0\n");
        }
        else
        {
            uart_nozzle_tx_hwsw_ver_get();
            MYSERIAL1.printLine("echo:M3011 uart_nozzle_tx_sw_ver_get\n");
        }
        break;
    }
}

// sn w/r, M3013 W0123456789abcdef / M3013 R0
void GcodeSuite::M3013(void)
{
    uint8_t n = 0;
    uint8_t *ptr;

    ptr = (uint8_t *)strchr(parser.command_ptr, 'W');
    if (ptr != 0)
    {
        ptr += 1;
        uart_nozzle_tx_multi_data(GCP_CMD_29_SN_WRITE, ptr, 16);
        MYSERIAL1.printLine("echo:M3013 W%s\n", ptr);
        return;
    }

    ptr = (uint8_t *)strchr(parser.command_ptr, 'R'); // E
    if (ptr != 0)
    {
        ptr += 1;
        n = ptr[0] - '0';
        uart_nozzle_tx_single_data(GCP_CMD_30_SN_READ, n);
        MYSERIAL1.printLine("echo:M3013 R%d\n", n);
    }
}

// heater fan io ctrl, M3015 Sx
void GcodeSuite::M3015(void)
{
    uint8_t n = 0;
    uint8_t *ptr;

    ptr = (uint8_t *)strchr(parser.command_ptr, 'S');
    if (ptr != 0)
    {
        ptr += 1;
        n = ptr[0] - '0';
        uart_nozzle_tx_multi_data(GCP_CMD_F4_HEATER_FAN_IO_CTRL, &n, 1);
        MYSERIAL1.printLine("echo:M3015 S%s\n", ptr);
        return;
    }
}

void GcodeSuite::M106_add_on(void)
{
    uint8_t on = 1;
    uart_nozzle_tx_multi_data(GCP_CMD_F4_HEATER_FAN_IO_CTRL, &on, 1);
}

void GcodeSuite::M107_add_on(void)
{
    uint8_t on = 0;
    uart_nozzle_tx_multi_data(GCP_CMD_F4_HEATER_FAN_IO_CTRL, &on, 1);
}

void GcodeSuite::M3001_add_on()
{
    uart_nozzle_tx_multi_data(GCP_CMD_45_M3001_DEAL, 0, 0);
    MYSERIAL1.printLine("echo:M3001 R\n");
}

void GcodeSuite::M3002_add_on()
{
    uart_nozzle_tx_multi_data(GCP_CMD_46_M3002_DEAL, 0, 0);
    MYSERIAL1.printLine("echo:M3002 R\n");
}

void GcodeSuite::M3003_add_on()
{
    MYSERIAL2.printLine("OK\r\n");
}

void GcodeSuite::M3020()
{
    if (!IS_new_nozzle_board())
        return;
    if (parser.seen('V'))
    {
        anker_probe_set.leveing_value = parser.value_int();
        uart_nozzle_tx_probe_leveling_val_set(anker_probe_set.leveing_value);
        MYSERIAL2.printLine("echo:M3020 V%d\r\n", anker_probe_set.leveing_value);
        settings.save();
        return;
    }
    uart_nozzle_tx_probe_leveling_val_get();
}

#endif
