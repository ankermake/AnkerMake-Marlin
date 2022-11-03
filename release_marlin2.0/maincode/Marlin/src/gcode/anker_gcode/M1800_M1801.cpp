#include "../gcode.h"
#include "../../inc/MarlinConfig.h"
#include "../queue.h"           // for getting the command port
#include "../../feature/anker/anker_log_debug.h"
#if ENABLED(ANKER_LOG_DEBUG)

void GcodeSuite::M1800()
{
  set_anker_debug_flag(1);
}

void GcodeSuite::M1801()
{
  set_anker_debug_flag(0);
}

#endif
