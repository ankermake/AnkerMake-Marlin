#pragma once

#include "../../inc/ANKER_Config.h"

#if ADAPT_DETACHED_NOZZLE

#include "uart_nozzle_tx.h"
#include <string.h>

#define NOZZLE_TYPE_NEW 0 // >= v8111-auto-level-v0_7
#define NOZZLE_TYPE_OLD 1 // <= v8111-auto-level-v0_5

extern "C"
{
    typedef struct
    {
        uint8_t hw_version[3];
        uint8_t sw_version[3];

        uint8_t material_sta;
    } nozzle_t;

    extern nozzle_t nozzle;
    const float* Get_NOZZLE_TO_PROBE_OFFSET(void);
    bool IS_new_nozzle_board(void);
    void uart_nozzle_init(void);
    void uart_nozzle_get_heartbeat_flag(void);
    void uart_nozzle_polling(void);

    void uart_nozzle_tx_probe_val(uint16_t val);
}

extern uint8_t nozzle_board_type;

#endif
