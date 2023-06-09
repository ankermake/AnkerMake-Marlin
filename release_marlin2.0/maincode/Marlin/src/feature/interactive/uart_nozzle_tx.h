#include "../../inc/ANKER_Config.h"

#if ADAPT_DETACHED_NOZZLE

#pragma once

extern "C"
{
#include <stdint.h>

    void uart_nozzle_tx_single_data(uint8_t cmd, uint8_t dat);
    void uart_nozzle_tx_hwsw_ver_get(void);
    void uart_nozzle_tx_probe_val(uint16_t val);
    void uart_nozzle_tx_show_adc_value_on();
    void uart_nozzle_tx_show_adc_value_off();
    void uart_nozzle_tx_pid_autotune(uint16_t temp, uint16_t ncycles);
    void uart_nozzle_tx_probe_val_get();
    void uart_nozzle_tx_probe_leveling_val_set(int16_t val);
    void uart_nozzle_tx_probe_leveling_val_get(void);
    void uart_nozzle_tx_multi_data(uint8_t cmd, uint8_t *buf, uint16_t size);
    void uart_nozzle_tx_auto_offset_start(void);
}

#endif
