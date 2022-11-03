/*
 * @Author       : Harley
 * @Date         : 2022-05-28 10:50:23
 * @LastEditors  : Harley
 * @LastEditTime : 2022-05-28 14:03:20
 * @Description  :
 */
#include "anker_z_sensorless.h"

#if ENABLED(ANKER_MAKE_API)
 #include "../../module/stepper/trinamic.h"

 #if ENABLED(ANKER_NOZZLE_BOARD)
   #include "anker_nozzle_board.h"
 #endif

 #if ENABLED(USE_Z_SENSORLESS)
  Use_Z_Sensorless use_z_sensorless;
   
   u_int16_t Use_Z_Sensorless::z1_stall_value=0;
   u_int16_t Use_Z_Sensorless::z2_stall_value=0;

   void Use_Z_Sensorless:: init()
   {
      stepperZ.anker_homing_threshold(use_z_sensorless.z1_stall_value);
      #ifdef ANKER_Z2_STALL_SENSITIVITY
       stepperZ2.anker_homing_threshold(use_z_sensorless.z2_stall_value); 
      #endif
   }
   void Use_Z_Sensorless:: reset()
   {
      use_z_sensorless.z1_stall_value=ANKER_Z_STALL_SENSITIVITY;
      #ifdef ANKER_Z2_STALL_SENSITIVITY
        use_z_sensorless.z2_stall_value=ANKER_Z2_STALL_SENSITIVITY;
      #endif
   }
   void Use_Z_Sensorless::set_z1_value(u_int16_t z1_value)
   {
       use_z_sensorless.z1_stall_value=z1_value;
       stepperZ.anker_homing_threshold(use_z_sensorless.z1_stall_value);
   }
   #ifdef ANKER_Z2_STALL_SENSITIVITY
   void Use_Z_Sensorless::set_z2_value(u_int16_t z2_value)
   {
       use_z_sensorless.z2_stall_value=z2_value;
       stepperZ2.anker_homing_threshold(use_z_sensorless.z2_stall_value);
   }
   #endif
   void Use_Z_Sensorless::report()
   {
         SERIAL_ECHO(" z1-stall:");
         SERIAL_ECHO(use_z_sensorless.z1_stall_value);
         SERIAL_ECHO(" \r\n");
         #ifdef ANKER_Z2_STALL_SENSITIVITY
            SERIAL_ECHO(" z2-stall:");
            SERIAL_ECHO(use_z_sensorless.z2_stall_value);
            SERIAL_ECHO(" \r\n");
         #endif
   }
   void Use_Z_Sensorless::z_sensorless_kill(u_int8_t sta)
   {
      if((sta==Z_SENSORLESS_KILL_STA)&&(get_anker_nozzle_board_info()->is_z_sensorless))
      {
         SERIAL_ECHO(Z_SENSORLESS_KILL_MSG);
         kill();
      }
   }
 #endif

#endif
