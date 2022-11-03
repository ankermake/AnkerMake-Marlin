#include "../../inc/MarlinConfig.h"
#if ENABLED(ANKER_MAKE_API)

#include "../../core/serial.h"
#include "../../gcode/queue.h"
#include "../../gcode/gcode.h"
#include "../../module/planner.h"

#if ENABLED(ANKER_LOG_DEBUG)
int anker_debug_flag;


void anker_check_block_buf(void)
{
  millis_t ms = millis();
  static millis_t next_report_ms = millis() + 5000;

  ms = millis();
  if(ms > next_report_ms)
  {
    next_report_ms = millis() + 5000;
    SERIAL_ECHOLNPAIR("next_report_ms:",next_report_ms);
    SERIAL_ECHOLNPAIR("anker_debug_flag:",anker_debug_flag);
    if(queue.ring_buffer.empty() && anker_debug_flag)
    {
      SERIAL_ECHOLN("queue.ring_buffer empty");
    }
    if(planner.get_current_block() == NULL  && anker_debug_flag )
    {
      SERIAL_ECHOLN("current_block NULL");
    }
   }
}
void set_anker_debug_flag(int value)
{
    anker_debug_flag=value;
}
int get_anker_debug_flag()
{
    return anker_debug_flag;
}
#endif
#endif
