#include "../gcode.h"
#include "../../inc/MarlinConfig.h"
#include "../queue.h"           // for getting the command port
#include "../../module/planner.h"
#include "../../feature/anker/anker_homing.h"
#include "../../feature/anker/anker_z_sensorless.h"
#include "../../feature/anker/board_configure.h"
#include "../../module/motion.h"
#include "../../module/stepper.h"
#include "../../module/endstops.h"

#if ENABLED(PROVE_CONTROL)
    
void GcodeSuite::M2000() {
    if (parser.seen('S'))
    {
       if(parser.value_bool())
       {
        digitalWrite(PROVE_CONTROL_PIN,PROVE_CONTROL_STATE);
        SERIAL_ECHOLNPGM("PROVE_CONTROL_PIN:OPEN!"); 
       }
       else
       {
        digitalWrite(PROVE_CONTROL_PIN,!PROVE_CONTROL_STATE);
        SERIAL_ECHOLNPGM("PROVE_CONTROL_PIN:CLOSE!"); 
       }
    }
    else
    {
       if(READ(PROVE_CONTROL_PIN)==PROVE_CONTROL_STATE)
       {
         SERIAL_ECHOLNPGM("PROVE_CONTROL is OPEN!");  
       }
       else 
       {
        SERIAL_ECHOLNPGM("PROVE_CONTROL is CLOSE!");  
       }
    }
}
#endif


#if ENABLED(READ_BUFFER_SIZE)
   void GcodeSuite::M2001()
   {
     SERIAL_ECHO("BLOCK_BUFFER_SIZE: ");
     SERIAL_ECHO(BLOCK_BUFFER_SIZE);
     SERIAL_ECHO("\r\nMAX_CMD_SIZE: ");
     SERIAL_ECHO(MAX_CMD_SIZE);
     SERIAL_ECHO("\r\nBUFSIZE: ");
     SERIAL_ECHO(BUFSIZE);
     SERIAL_ECHO("\r\nSLOWDOWN_DIVISOR: ");
     SERIAL_ECHO(SLOWDOWN_DIVISOR);
     #ifdef XY_FREQUENCY_LIMIT
      SERIAL_ECHO("\r\nXY_FREQUENCY_LIMIT: ");
      SERIAL_ECHO(planner.xy_freq_limit_hz);
      SERIAL_ECHO("\r\nXY_FREQUENCY_MIN_PERCENT: ");
      SERIAL_ECHO(planner.xy_freq_min_speed_factor);
     #endif
     SERIAL_ECHO("\r\n");  
     SERIAL_ECHO("\r\n");     
   }
#endif

#if ENABLED(BOARD_CONFIGURE)
   void GcodeSuite::M2002()
   {
      SERIAL_ECHO("Board chek \r\n");
      SERIAL_ECHO("adc1: ");
      SERIAL_ECHO(board_configure.adc1);
      SERIAL_ECHO("\r\n");
      SERIAL_ECHO("adc2: ");
      SERIAL_ECHO(board_configure.adc2);
      SERIAL_ECHO("\r\n");
   }
#endif

#if ENABLED(USE_Z_SENSORLESS)

   void GcodeSuite::M2003()
   {
      bool report = true;
      const uint8_t index = parser.byteval('I');
      LOOP_LINEAR_AXES(i) if (parser.seen(AXIS_CHAR(i))) {
      const int16_t value = parser.value_int();
        report=false;
      switch (i) {
          case Z_AXIS:
            if (index == 1) 
            {
              use_z_sensorless.set_z1_value(value);
            }
            else if (index == 2) 
            {
              #ifdef ANKER_Z2_STALL_SENSITIVITY
                use_z_sensorless.set_z2_value(value);
              #endif
            }
            else if(index == 0)
            {
              use_z_sensorless.set_z1_value(value);
              #ifdef ANKER_Z2_STALL_SENSITIVITY
               use_z_sensorless.set_z2_value(value);
              #endif
            }
            break;
       }
    }

    if (report) 
    {
        use_z_sensorless.report();
    }
   }
#endif


#if ENABLED(EVT_HOMING_5X)
   void GcodeSuite::M2004()
   {
    #if ENABLED(NO_MOTION_BEFORE_HOMING)
     if (parser.seen('S'))
    {
       if(parser.value_bool())
       {
        anker_homing.no_check_all_axis=true;
        SERIAL_ECHOLNPGM("no_check_all_axis:OPEN!"); 
       }
       else
       {
        anker_homing.no_check_all_axis=false;  
        SERIAL_ECHOLNPGM("no_check_all_axis:CLOSE!"); 
       }
    }
    else
    {
       if(anker_homing.no_check_all_axis)
       {
        SERIAL_ECHOLNPGM("no_check_all_axis:OPEN!"); 
       }
       else 
       {
        SERIAL_ECHOLNPGM("no_check_all_axis:CLOSE!"); 
       }
    }
    #endif
   }  
#endif

#if ENABLED(USE_Z_SENSORLESS)
      void GcodeSuite::M2005()
      {
      if(anker_homing.get_first_end_z_axis()==Z_AXIS_IS_Z1)
       {
        SERIAL_ECHOLNPGM("first_end_z_axis:Z1!"); 
       }
       else if(anker_homing.get_first_end_z_axis()==Z_AXIS_IS_Z2)
       {
        SERIAL_ECHOLNPGM("first_end_z_axis:Z2!"); 
       }
       else if(anker_homing.get_first_end_z_axis()==Z_AXIS_IDLE)
       {
        SERIAL_ECHOLNPGM("first_end_z_axis:IDLE!");           
       }
      }

      void GcodeSuite::M2006()//Auto-Proof Threshold
      {
         use_z_sensorless.set_z1_value(80);//Set an insensitive threshold so that it must return to zero
         use_z_sensorless.set_z2_value(80);

         gcode.process_subcommands_now_P("G28 Z\n");
         
      }
#endif

#if ENABLED(TMC_AUTO_CONFIG)
   void GcodeSuite::M2007()
   {
       stepperX.rms_current(800, 1);
       stepperY.rms_current(800, 1);
       stepperX.set_pwm_thrs(301);
       stepperY.set_pwm_thrs(301);
        TMC2208_n::PWMCONF_t pwmconf{0};
        pwmconf.pwm_lim = 12;
        pwmconf.pwm_reg = 8;
        pwmconf.pwm_autograd = true;
        pwmconf.pwm_autoscale = true;
        pwmconf.pwm_freq = 0b01;
        pwmconf.pwm_grad = 14;
        pwmconf.pwm_ofs = 36;
        stepperX.PWMCONF(pwmconf.sr);
        ENABLE_AXIS_X();
        ENABLE_AXIS_Y();
        _delay_ms(200);

        SERIAL_ECHO("PER-XPWM_SCALE_AUTO: ");
        SERIAL_ECHO(stepperX.pwm_scale_auto());
        SERIAL_ECHO("\r\n");
        SERIAL_ECHO("PER-YPWM_SCALE_AUTO: ");
        SERIAL_ECHO(stepperY.pwm_scale_auto());
        SERIAL_ECHO("\r\n");
        gcode.process_subcommands_now_P("M204 P100 R100 T100\n");
        gcode.process_subcommands_now_P("G1 X10 F60\n");
        gcode.process_subcommands_now_P("G1 X20 F7500\n");
        SERIAL_ECHO("XPWM_SCALE_AUTO: ");
        SERIAL_ECHO(stepperX.pwm_scale_auto());
        SERIAL_ECHO("\r\n");
        SERIAL_ECHO("YPWM_SCALE_AUTO: ");
        SERIAL_ECHO(stepperY.pwm_scale_auto());
        SERIAL_ECHO("\r\n");
   }
#endif

#if ENABLED(ANKER_EXTRUDERS_RECEIVE)
    void GcodeSuite::M2008()
         {
           if (parser.seen('S'))
            {
               if(parser.value_byte()==0)
               {
                 gcode.process_subcommands_now_P(PSTR("G1 X0\n"));
               }
               else if(parser.value_byte()==1)
               {
                 gcode.process_subcommands_now_P(PSTR("G1 X17\n"));
               }
            }
         }       
#endif

     

