#include "../../inc/MarlinConfig.h"
#if ENABLED(HANDSHAKE)
    #include "handshake.h"

    HandShake hand_shake;

    void HandShake::init()
    {
     pinMode(HANDSHAKE_SDO,INPUT_FLOATING);
     pinMode(HEATER_EN_PIN,OUTPUT);
     digitalWrite(HEATER_EN_PIN,!HEATER_EN_STATE);

     if(digitalRead(HANDSHAKE_SDO))
     {
        digitalWrite(HEATER_EN_PIN,HEATER_EN_STATE);
     }
     else
     {
        SERIAL_ECHO("echo:The nozzle is connected incorrectly");
        SERIAL_ECHO("\r\n");
     }
    
    }

    void HandShake::handshake_test()
    {
      // u8 status=0;
      // switch (status)
      // {
      // case 0:
       
      //   break;
      // case 1:
       
      //   break;
      // case 2:
       
      //   break;
      
      // default:
      //   break;
      // }
       
      // pinMode(HANDSHAKE_SDO,INPUT);
      // digitalWrite(HANDSHAKE_SDO,!HANDSHAKE_STATE);
      // digitalWrite(PROVE_CONTROL_PIN,PROVE_CONTROL_STATE);//open power
      // for(uint16_t time=0;time<20;time++)
      // {
      //   digitalWrite(HANDSHAKE_SDO,!HANDSHAKE_STATE);
      //   _delay_ms(10);
      //   digitalWrite(HANDSHAKE_SDO,HANDSHAKE_STATE);
      //   _delay_ms(10); 
      // }
      // pinMode(HANDSHAKE_SDO,INPUT);
    }

    void HandShake::check()
    {
      millis_t ms = millis();
      static millis_t next_report_ms = millis() + HANDSHAKE_TIME;

      ms = millis();
      if(ms > next_report_ms)
      {
        next_report_ms = millis() + HANDSHAKE_TIME;
         pinMode(HANDSHAKE_SDO,INPUT_FLOATING);
        if(digitalRead(HANDSHAKE_SDO))
          {
            digitalWrite(HEATER_EN_PIN,HEATER_EN_STATE);
          }
          else
          {
            digitalWrite(HEATER_EN_PIN,!HEATER_EN_STATE);
         
            // SERIAL_ECHO("echo:The nozzle is connected incorrectly");
                 SERIAL_ECHO("echo:The nozzle:");
            SERIAL_ECHO(digitalRead(HANDSHAKE_SDO));
            SERIAL_ECHO("\r\n");
          }
      }
    }

#endif
