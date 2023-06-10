/*
 * @Author         : winter
 * @Date           : 2022-08-23 11:42:50
 * @LastEditors    : winter
 * @LastEditTime   : 2022-08-25 15:10:45
 * @Description    :
 */
#include "anker_z_offset.h"

#if ENABLED(ANKER_MAKE_API)

#include "../../gcode/gcode.h"
#include "../../module/probe.h"
#include "../interactive/uart_nozzle_rx.h"

Anker_Probe_set anker_probe_set;
int16_t Anker_Probe_set::homing_value = HOMING_PROBE_VALUE;
int16_t Anker_Probe_set::leveing_value = LEVEING_PROBE_VALUE;
uint16_t Anker_Probe_set::delay = LEVEING_PROBE_DELAY;
uint8_t Anker_Probe_set::run_step = 0;
bool Anker_Probe_set::auto_run_flag = false;
bool Anker_Probe_set::point_test_flag = false;
xy_pos_t Anker_Probe_set::xy[5];

void Anker_Probe_set::probe_start(uint16_t value)
{
#if ADAPT_DETACHED_NOZZLE
    safe_delay(anker_probe_set.delay);
    uart_nozzle_tx_probe_val(value);
    SERIAL_ECHO("probe_start\r\n");
#else
    MYSERIAL1.printf("M2012 S%d\n", value);
#endif
}

void Anker_Probe_set::reset_value()
{
    anker_probe_set.homing_value = HOMING_PROBE_VALUE;
    anker_probe_set.leveing_value = LEVEING_PROBE_VALUE;
}

void Anker_Probe_set::report_value()
{
    SERIAL_ECHO("anker probe homing value: ");
    SERIAL_ECHO(anker_probe_set.homing_value);
    SERIAL_ECHO("\r\nanker probe leveing value: ");
    SERIAL_ECHO(anker_probe_set.leveing_value);
    SERIAL_ECHO("\r\n");
}

void Anker_Probe_set::get_probe_value()
{
    uart_nozzle_tx_probe_val_get();
}

void Anker_Probe_set::show_adc_value(bool show_adc)
{
    if (show_adc)
    {
        uart_nozzle_tx_show_adc_value_on();
    }
    else
    {
        uart_nozzle_tx_show_adc_value_off();
    }
}

void Anker_Probe_set::pid_autotune(uint16_t temp, uint16_t ncyclesc)
{
    uart_nozzle_tx_pid_autotune(temp, ncyclesc);
    SERIAL_ECHO("PID AUTOTUNE START \r\n");
}

void Anker_Probe_set::run()
{
    switch (anker_probe_set.run_step)
    {
    case 0:
        gcode.process_subcommands_now_P(PSTR("M104 S230\nM109 S230\n"));
        gcode.process_subcommands_now_P(PSTR("G28\n"));
        gcode.process_subcommands_now_P(PSTR("G1 Z80 F900\n"));
        gcode.process_subcommands_now_P(PSTR("G1 X110 Y110 F6000\n"));
        anker_probe_set.run_step = 1;
        break;
    case 1:
        gcode.process_subcommands_now_P(PSTR("G2001\n"));
        break;
    default:
        break;
    }
}

void Anker_Probe_set::home_delay()
{
    safe_delay(PROBE_READ_NUM_DELAY);
}

void Anker_Probe_set::auto_run()
{
    gcode.process_subcommands_now_P(PSTR("M104 S230\nM109 S230\n"));
    anker_probe_set.auto_run_flag = true;
    gcode.process_subcommands_now_P(PSTR("G28\n"));
}

void Anker_Probe_set::auto_run_send_start_info()
{
    safe_delay(anker_probe_set.delay);
    uart_nozzle_tx_auto_offset_start();
    SERIAL_ECHO("auto_run_send_start_info\r\n");
}

void Anker_Probe_set::point_test_ready()
{
    anker_probe_set.xy[0].x = 0;
    anker_probe_set.xy[0].y = 110;
    anker_probe_set.xy[1].x = 55;
    anker_probe_set.xy[1].y = 110;
    anker_probe_set.xy[2].x = 110;
    anker_probe_set.xy[2].y = 110;
    anker_probe_set.xy[3].x = 165;
    anker_probe_set.xy[3].y = 110;
    anker_probe_set.xy[4].x = 220;
    anker_probe_set.xy[4].y = 110;

    gcode.process_subcommands_now_P(PSTR("M104 S230\nM109 S230\n"));
    gcode.process_subcommands_now_P(PSTR("G28\n"));
    gcode.process_subcommands_now_P(PSTR("G1 Z80 F900\n"));
    gcode.process_subcommands_now_P(PSTR("G1 X110 Y110 F6000\n"));
    anker_probe_set.point_test_flag = true;
}

void Anker_Probe_set::point_test(uint8_t point)
{
    probe.probe_at_point(anker_probe_set.xy[point], PROBE_PT_RAISE, 0, true, false);
}

#endif /* ANKER_PROBE_SET */
