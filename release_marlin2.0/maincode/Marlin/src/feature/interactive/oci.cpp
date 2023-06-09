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
#include "oci.h"
#include "protocol.h"
#include "soft_timer.h"

#define OCI_IN_PIN            PC6
#define OCI_CLR_PIN           PD10
#define OCI_HOTBED_PWR_PIN    HEATER_BED_CTRL_2_PIN
#define OCI_NMOS_CHK_PIN      PE15
#define OCI_HEATER_HOTBED_PIN HEATER_BED_PIN

    typedef struct
    {
        soft_timer_t mos_soft_timer;
        // soft_timer_t latch_soft_timer;
        // uint8_t mutex;
    } oci_t;

    oci_t oci;

#define OCI_FEED_READ() READ(OCI_NMOS_CHK_PIN)

    void oci_latch_clear(void)
    {
        OUT_WRITE(OCI_CLR_PIN, LOW);
        safe_delay(1);
        OUT_WRITE(OCI_CLR_PIN, HIGH);
    }

    // static void oci_latch_re_enable_callback(void)
    // {
    //     oci_latch_clear();
    //     oci.mutex = 0;
    // }

    // static void oci_in_isr(void)
    // {
    //     if (oci.mutex != 0)
    //         return;

    //     oci.mutex = 1;
    //     soft_timer_start(&oci.latch_soft_timer);
    //     MYSERIAL2.printLine("hotbed oci trigger!\n");
    // }

    static void oci_delay(void)
    {
        volatile uint32_t count = 500;
        while (count--)
            ;
    }

    // static void oci_mos_check_callback(void)
    // {
    //     uint8_t level;

    //     if (oci.mutex == 1)
    //         return;

    //     if (READ_OUT(OCI_HOTBED_PWR_PIN) == 0)
    //         return;

    //     if (Temperature::temp_bed.target != 0)
    //         return;

    //     DISABLE_TEMPERATURE_INTERRUPT();
    //     level = READ_OUT(OCI_HEATER_HOTBED_PIN);
    //     OUT_WRITE(OCI_HEATER_HOTBED_PIN, HIGH);
    //     oci_delay();
    //     if (OCI_FEED_READ() == 1)
    //     {
    //         safe_delay(5);
    //         if (OCI_FEED_READ() == 1)
    //         {
    //             OUT_WRITE(OCI_HOTBED_PWR_PIN, LOW);
    //             MYSERIAL2.printLine("Error:Demage:1 bed\n");
    //         }
    //     }

    //     OUT_WRITE(OCI_HEATER_HOTBED_PIN, level);
    //     ENABLE_TEMPERATURE_INTERRUPT();
    // }

    typedef struct
    {
        uint8_t flg;
        uint32_t start_ms;
    } hotbed_mos_check_t;
    hotbed_mos_check_t hotbed_mos_check;

    static void oci_mos_check_callback(void)
    {
        uint8_t level;

        // if (oci.mutex == 1)
        // {
        //     return;
        // }

        if (Temperature::temp_bed.target != 0)
        {
            return;
        }

        // if (READ_OUT(OCI_HOTBED_PWR_PIN) == 0)
        // {
        //     return;
        // }

        OUT_WRITE(OCI_CLR_PIN, LOW); // oci check disable

        DISABLE_TEMPERATURE_INTERRUPT();
        level = READ_OUT(OCI_HEATER_HOTBED_PIN);
        OUT_WRITE(OCI_HEATER_HOTBED_PIN, HIGH);
        oci_delay();
        if (OCI_FEED_READ() == 1)
        {
            safe_delay(5);
            if (OCI_FEED_READ() == 1)
            {
                // OUT_WRITE(OCI_HOTBED_PWR_PIN, LOW);
                // MYSERIAL2.printLine("Error:Demage:1 bed\n");
                if (hotbed_mos_check.flg == 0)
                {
                    hotbed_mos_check.flg = 1;
                    hotbed_mos_check.start_ms = getCurrentMillis();
                }
            }
        }

        OUT_WRITE(OCI_HEATER_HOTBED_PIN, level);
        ENABLE_TEMPERATURE_INTERRUPT();

        OUT_WRITE(OCI_CLR_PIN, HIGH); // oci check enable
    }

    uint8_t fatal_err;
    void hotbed_mos_err_polling(void)
    {
        if (hotbed_mos_check.flg == 0)
            return;

        if (int(getCurrentMillis() - hotbed_mos_check.start_ms) > 5000)
        {
            fatal_err = 1;
            MYSERIAL2.printf("TempErrorCode:1011\r\n");
            MYSERIAL2.printLine("Error:Demage:1 bed\n");
            hotbed_mos_check.flg = 0;
            OUT_WRITE(OCI_HOTBED_PWR_PIN, LOW);
        }
    }

    static uint8_t hw_ver;

    uint8_t hw_ver_read(void)
    {
        return hw_ver;
    }

    void hw_select(void)
    {
        SET_INPUT_PULLDOWN(OCI_IN_PIN);
        _delay_ms(10);
        hw_ver = READ(OCI_IN_PIN) ? 1 : 0;
        MYSERIAL1.printLine("hw_ver = %d\n", hw_ver);
    }

    void oci_init(void)
    {
        if (hw_ver_read() == 0)
            return;

        SET_INPUT_PULLUP(OCI_IN_PIN); // falling edge trigger

        // attachInterrupt(digitalPinToInterrupt(OCI_IN_PIN), oci_in_isr, FALLING);

        soft_timer_init(&oci.mos_soft_timer, 100, 0, oci_mos_check_callback);
        soft_timer_start(&oci.mos_soft_timer);

        // soft_timer_init(&oci.latch_soft_timer, 100, 1, oci_latch_re_enable_callback);
    }
}

#endif
