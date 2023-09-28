#include "../../inc/ANKER_Config.h"
#include "../../inc/MarlinConfig.h"
#include "../../core/macros.h"

#if ADAPT_DETACHED_NOZZLE

#pragma once

enum anker_production_mode_parm
{
    STA_00_OFF = 0,
    STA_01_ON  = 1, // only probing
    STA_02_ON  = 2, // only heater
};

enum anker_pointtype
{
    POINT_UNKNOW = 0,
    POINT_G36 = 1,
    POINT_G28 = 2,
    POINT_G29 = 3,
    POINT_END,
};

enum anker_probeactiontype
{
    PROBE_UNKNOW            = 0,
    PROBE_START             = 1,
    PROBE_ZERO_POINT_OK     = 2,
    PROBE_ZERO_POINT_FAILED = 3,
    PROBE_M3001             = 4,
    PROBE_M3002             = 5,
    PROBE_TRIGGER_NORMAL    = 6,
    PROBE_TRIGGER_IDLE      = 7,
    PROBE_END,
};

enum anker_production_mode
{
    PRODUCTION_NORMAL_MODE = 0, // normal mode
    PRODUCTION_TEST_MODE   = 1, // production test mode
    PRODUCTION_END,
};

enum anker_overpressure_type
{
    TRIGGER_5MS = 0,// working in normal mode
    TRIGGER_HPF = 1, // working in normal mode
    TRIGGER_OVERPRESSURE = 2, // working in production test mode
    TRIGGER_IDLE = 3,
    TRIGGER_NORMAL_THRESHOLD = 4, // working in normal mode
    TRIGGER_TEST_THRESHOLD = 5, // working in production test mode
    TRIGGER_AREA_CALC = 6, // 
    trigger_END,
};

enum anker_overpressure_trigger_type
{
    OVERPRESSURE_TRIGGER_CLOSED = 0, // working in production test mode
    OVERPRESSURE_TRIGGER_OPEN = 5, // working in production test mode
};

enum anker_data_transmission_format_type
{
    PARM_MODIFICATION = 1, 
    CANCEL_ALALRN = 2,
    DTF_NONE,
};

extern "C"
{
#include <stdint.h>

    typedef struct _production_mode_t
    {
        uint8_t mode;  // mode = PRODUCTION_NORMAL_MODE/PRODUCTION_TEST_MODE
        uint8_t parm;  // parm = STA_00_OFF/STA_01_ON/STA_02_ON
        uint8_t type;  // type = G28/G36/G29
        uint8_t point; // point= position or count in different modes
        TERN_(ANKER_OVERPRESSURE_REPORT, uint8_t overpressure_trigger); // Marlin Probing Failed / Error:Homing Failed / Error:Adjustment range over
        int32_t threshold;
        int32_t current_diff; //current_val - initial_val
        millis_t rx_timeout;
        volatile uint8_t rx_type; // rx_type = anker_probeactiontype
        volatile bool rx_ack; // Feedback query response situation
    } production_mode_t;
    typedef struct _clogged_nozzle_t
    {
        int8_t type;
        int8_t countclear;
        int8_t enable;
        int32_t threshold;
    } clogged_nozzle_t;



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
    void uart_nozzle_tx_point_type(uint8_t type, uint8_t point); // Notify the nozzle board of the position under different commands.
    void uart_nozzle_tx_production_mode(const uint8_t mode, const uint8_t parm); // Switch between production test mode and normal mode.
    void uart_nozzle_tx_clogged_nozzle(const clogged_nozzle_t clogged_nozzle); // Clogged nozzle config
}
extern production_mode_t production_mode;
#define POINT_TYPE_STRING   (production_mode.type == POINT_G28 ? "G28": (production_mode.type == POINT_G29 ? "G29" : (production_mode.type == POINT_G36 ? "G36" : "unknow")))
#define POINT_TYPE_POSITION (production_mode.point)
#define ANKER_NORMAL_MODE() (production_mode.mode == PRODUCTION_NORMAL_MODE)
#define ANKER_TEST_MODE()   (production_mode.mode == PRODUCTION_TEST_MODE)
#if ENABLED(ANKER_OVERPRESSURE_REPORT)
#define ANKER_OVERPRESSURE_TRIGGER() (ANKER_TEST_MODE() && (production_mode.overpressure_trigger == OVERPRESSURE_TRIGGER_OPEN))
#define ANKER_CLOSED_OVERPRESSURE_TRIGGER() (production_mode.overpressure_trigger = OVERPRESSURE_TRIGGER_CLOSED)
#else
#define ANKER_OVERPRESSURE_TRIGGER() 0
#define ANKER_CLOSED_OVERPRESSURE_TRIGGER() 
#endif

#define PROBE_ACTION_STRING(type)   ( type == PROBE_START ? "start":\
                                     (type == PROBE_ZERO_POINT_OK ? "zero_ok" :\
                                     (type == PROBE_ZERO_POINT_FAILED ? "zero_failed":\
                                     (type == PROBE_M3001 ? "M3001":\
                                     (type == PROBE_M3002 ? "M3002": "unknow")))))
#define PROBE_IS_START_SIGNAL()      (PROBE_START == production_mode.rx_type\
                                      || PROBE_ZERO_POINT_OK == production_mode.rx_type\
                                      || PROBE_ZERO_POINT_FAILED == production_mode.rx_type)

#endif
