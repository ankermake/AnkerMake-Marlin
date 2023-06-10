#include "../../inc/ANKER_Config.h"

#if ADAPT_DETACHED_NOZZLE

#pragma once

extern "C"
{
#include <stdint.h>

    void oci_init(void);

    void oci_latch_clear(void);

    void hw_select(void);

    uint8_t hw_ver_read(void);

    void hotbed_mos_err_polling(void);

    extern uint8_t fatal_err;
}

#endif
