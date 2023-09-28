/*
 * @Author       : harley
 * @Date         : 2022-04-27 20:35:23
 * @LastEditors: winter.tian
 * @LastEditTime: 2023-03-01 12:17:41
 * @Description  :
 */
#include "../../inc/MarlinConfig.h"

#if ENABLED(ANKER_ANLIGN)
  #include "anker_align.h"
  #include "../../module/stepper.h"
  #include "../../module/probe.h"
  #include "../../gcode/gcode.h"
  #include "../../module/temperature.h"
  #if ENABLED(ADAPT_DETACHED_NOZZLE)
   #include "../interactive/uart_nozzle_rx.h"
  #endif 
  #if ADAPT_DETACHED_NOZZLE
  #include "../interactive/uart_nozzle_tx.h"
  #endif
  #if ENABLED(EVT_HOMING_5X)
  #include "../../feature/anker/anker_homing.h"
  #endif

  Anker_Align anker_align;
    
    float Anker_Align::eeprom_z1_value=0;
    float Anker_Align::eeprom_z2_value=0;
    bool Anker_Align::is_g36_cmd_executing=false;
    float Anker_Align::z1_value=0;
    float Anker_Align::z2_value=0;
    //uint8_t Anker_Align::anker_is_leveing=0;
    xy_pos_t Anker_Align::xy[NUM_Z_STEPPER_DRIVERS];

    void Anker_Align:: init(void)
    {
      anker_align.xy[0].x=PROBING_MARGIN;
      anker_align.xy[0].y=110;
      anker_align.xy[1].x=X_BED_SIZE-PROBING_MARGIN;
      anker_align.xy[1].y=110;
      #ifdef  ALIGN_PER_RESET
      anker_align.reset();
      #endif
    }
    void Anker_Align:: add_z_value(float value,uint8_t choose)
    {
       stepper.set_separate_multi_axis(true);
       // Lock all steppers except one
       stepper.set_all_z_lock(true, choose);
       // Do a move to correct part of the misalignment for the current stepper
       do_blocking_move_to_z(current_position.z+value);
 
       // Back to normal stepper operations
       stepper.set_all_z_lock(false);
       stepper.set_separate_multi_axis(false);   
    }
    void Anker_Align:: add_z1_value(float value)
    {
       anker_align.z1_value+=value;
       anker_align.add_z_value(value,0);
    }
    void Anker_Align:: add_z2_value(float value)
    {
       anker_align.z2_value+=value;
       anker_align.add_z_value(value,1);  
    }
    void Anker_Align:: add_z1_value_no_save(float value)
    {
       anker_align.add_z_value(value,0); 
    }
    void Anker_Align:: add_z2_value_no_save(float value)
    {
       anker_align.add_z_value(value,1);
    }
    void Anker_Align::run_align(void)
    {
       float value=0;
       value=anker_align.z1_value;
       anker_align.z1_value=0;
       anker_align.add_z1_value(value);  
       value=anker_align.z2_value;
       anker_align.z2_value=0;
       anker_align.add_z2_value(value);
    }
    void Anker_Align::reset(void)
    {
       anker_align.z1_value=0;
       anker_align.z2_value=0;
       anker_align.run_align();
    }

    void Anker_Align::auto_align(void)
    {
     uint16_t num=0;
     is_g36_cmd_executing = true;
     const ProbePtRaise raise_after =  PROBE_PT_RAISE;
     const float __align_allowed = TERN(ADAPT_DETACHED_NOZZLE, IS_new_nozzle_board() ? NOZZLE_TYPE_NEW_ANLIGN_ALLOWED : ANLIGN_ALLOWED, ANLIGN_ALLOWED);
     anker_align.init();
     gcode.process_subcommands_now_P(PSTR("G28"));
     for(num=0;num<ANLIGN_NUM;num++)
     {
         do_blocking_move_to_z(current_position.z+ANLIGN_RISE);

         TERN_(ADAPT_DETACHED_NOZZLE, if(IS_new_nozzle_board())uart_nozzle_tx_point_type(POINT_G36, num));

         const float z1 = probe.probe_at_point(anker_align.xy[0], raise_after, 0, true, false);
         if(isnan(z1))
         {
            SERIAL_ECHO("ok\r\n");
            SERIAL_ERROR_MSG("z1 is nan error!\r\n");
            SERIAL_ERROR_MSG(STR_ERR_PROBING_FAILED);
            kill();
         }
         do_blocking_move_to_z(current_position.z+ANLIGN_RISE);
         const float z2 = probe.probe_at_point(anker_align.xy[1], raise_after, 0, true, false);
         if(isnan(z2))
         {
            SERIAL_ECHO("ok\r\n");
            SERIAL_ERROR_MSG("z2 is nan error!\r\n");
            SERIAL_ERROR_MSG(STR_ERR_PROBING_FAILED);
            kill();
         }
         float rise_z=0;
         rise_z=(SCREW_DISTANCE/(anker_align.xy[1].x-anker_align.xy[0].x))*ABS(z1-z2);
         SERIAL_ECHO(" z1:");
         SERIAL_ECHO(z1);
         SERIAL_ECHO(" \r\n");
         SERIAL_ECHO(" z2:");
         SERIAL_ECHO(z2);
         SERIAL_ECHO(" \r\n");
         if(ABS(z1-z2)>ANLIGN_MAX_VALUE)
         {
            SERIAL_ECHO("ok\r\n");
            SERIAL_ERROR_MSG("Adjustment range over 2mm!!\r\n");
            SERIAL_ERROR_MSG(STR_ERR_PROBING_FAILED);
            kill();
         }
         if(ABS(z1-z2) <= __align_allowed)
         {
            anker_align.eeprom_z1_value = anker_align.z1_value;
            anker_align.eeprom_z2_value = anker_align.z2_value;
            is_g36_cmd_executing = false;
            SERIAL_ECHO("echo:anlign ok!\r\n");
            gcode.process_subcommands_now_P(PSTR("M500\n"));
            MYSERIAL2.printf("anker_align.eeprom_z1_value: %f\r\n", anker_align.eeprom_z1_value);
            MYSERIAL2.printf("anker_align.eeprom_z2_value: %f\r\n", anker_align.eeprom_z2_value);
            break;
         }
         else if(z1>z2)
         {
            anker_align.add_z1_value(rise_z);
         }
         else if(z2>z1)
         {
            anker_align.add_z2_value(rise_z);
         }
    
         anker_align.xy[0].x=PROBING_MARGIN;
         anker_align.xy[0].y+=3;
         anker_align.xy[1].x=X_BED_SIZE-PROBING_MARGIN;
         anker_align.xy[1].y+=3;

         if(num==(ANLIGN_NUM-1))
         {
           if(ABS(z1-z2) > __align_allowed)
           {
               SERIAL_ECHO("echo:Please check the Z-axis limit!\r\n");
               anker_align.reset();
           }
         }

         if(anker_align.is_g36_cmd_executing == false)
         {
            SERIAL_ECHO("echo:anker_align stop!\r\n");
            break;
         }
     }
        gcode.process_subcommands_now_P(PSTR("G2001\n"));
        TERN_(USE_Z_SENSORLESS, anker_homing.is_again_probe_homing = false);
    }


#endif

#if ENABLED(ANKER_ANLIGN_ONLY_Z)

  #include "anker_align.h"
  #include "../../module/stepper.h"
  #include "../../module/probe.h"
  #include "../../gcode/gcode.h"
  #include "../../module/temperature.h"

  Anker_Align_Only_Z anker_align_only_z;
  xy_pos_t Anker_Align_Only_Z::xy;

   void Anker_Align_Only_Z:: init(void)
   {
   // anker_align_only_z.xy[0].x=PROBING_MARGIN;
   // anker_align_only_z.xy[0].y=Y_BED_SIZE/2;
          anker_align_only_z.xy.x=190;
      anker_align_only_z.xy.y=100;
   }

 void Anker_Align_Only_Z::auto_run()
 {
     uint16_t num=0;
     const ProbePtRaise raise_after =  PROBE_PT_RAISE;
     gcode.process_subcommands_now_P(PSTR("G28"));
     anker_align_only_z.init();
     for(num=0;num<ANLIGN_NUM;num++)
     {
      do_blocking_move_to_z(current_position.z+ANLIGN_RISE);
      const float z1 = probe.probe_at_point(0,100, raise_after, 0, true, false);
      const float z2 = probe.probe_at_point(200,100, raise_after, 0, true, false);
         SERIAL_ECHO(" z1:");
         SERIAL_ECHO(z1);
         SERIAL_ECHO(" \r\n");
         SERIAL_ECHO(" z2:");
         SERIAL_ECHO(z2);
         SERIAL_ECHO(" \r\n");
     }
 }


#endif




