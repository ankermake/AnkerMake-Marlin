/*
 * @Author       : Harley
 * @Date         : 2022-05-28 10:50:23
 * @LastEditors  : Harley
 * @LastEditTime : 2022-05-28 14:03:20
 * @Description  :
 */
#include "../../inc/MarlinConfig.h"
// #include "../../"
#include "../../gcode/queue.h"

#if ENABLED(ANKER_MAKE_API)

#if ENABLED(USE_Z_SENSORLESS)

   #define Z_SENSORLESS_KILL_STA  0
   #define Z_SENSORLESS_KILL_MSG  "echo:Z homing foreign object detected!"

   class Use_Z_Sensorless {
    public:
       static u_int16_t z1_stall_value;
       static u_int16_t z2_stall_value;
       void init();
       void reset();
       void set_z1_value(u_int16_t z1_value);
       #ifdef ANKER_Z2_STALL_SENSITIVITY
         void set_z2_value(u_int16_t z2_value);
       #endif
       void report();
       void z_sensorless_kill(u_int8_t sta);
   };

  extern Use_Z_Sensorless use_z_sensorless;

#endif
#endif
