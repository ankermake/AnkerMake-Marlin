#include "../gcode.h"
#include "../../inc/MarlinConfig.h"
#include "../queue.h"           // for getting the command port
#include "../../feature/anker/anker_align.h" 

#if ENABLED(ANKER_ANLIGN_ONLY_Z)
  void GcodeSuite::G40()
   {
     anker_align_only_z.auto_run();
   }

#endif

