/*
 * @Author       : Anan
 * @Date         : 2023-05-06 19:02:21
 * @LastEditors  : Anan
 * @LastEditTime : 2023-05-26 19:40:44
 * @Description  :
 */

#include "../gcode.h"
#include "../../inc/MarlinConfig.h"
#include "../queue.h"           // for getting the command port

#if ENABLED(ANKER_MAKE_API)

#include "../../feature/interactive/uart_nozzle_tx.h"
#include "../../feature/interactive/protocol.h"



void GcodeSuite::M3030()
{// M3030 [C<log_type>] [T<point_type>] [P<point_number>]
  if (parser.seen('T'))
  {
    uint8_t point = 0; // (type, point)
    uint8_t type = parser.value_byte();
    if(parser.seen('P')){
      point = parser.value_byte();
    }
    uart_nozzle_tx_point_type(type, point);
    MYSERIAL2.printLine("echo:M3030 T%d P%d\n", type, point);
  }
  else if (parser.seen('C'))
  {
    uint8_t status = parser.value_byte();
    uart_nozzle_tx_single_data(GCP_CMD_47_DEBUG_LOG, status);
    MYSERIAL2.printLine("echo:M3030 C%d\n", status);
  }
}



void GcodeSuite::M3031()
{// M3031 [U<mode>] [P<nozzle_type>] [L<marlin_type>]
  
  if (parser.seen('U'))
  {
    switch (parser.value_int())
    {
      case PRODUCTION_NORMAL_MODE:
        {
          uart_nozzle_tx_production_mode(PRODUCTION_NORMAL_MODE, STA_00_OFF);
        }
        break;
        
      case PRODUCTION_TEST_MODE:
        {
          uint8_t parm = STA_00_OFF;
          if (parser.seen('P')){parm = parser.value_byte();}
          uart_nozzle_tx_production_mode(PRODUCTION_TEST_MODE, parm);
        }
        break;

      default:
        break;
    }
  }
}

#if ENABLED(ANKER_PROBE_DETECT_TIMES)
static xy_pos_t move_away[4] = {{-1.0, -1.0}, {2.0, 0.0}, {0.0, 2.0}, {-2.0, 0.0}};

void GcodeSuite::M3032()
{// M3032 [P<offset distance>]
  
  if (parser.seen('P'))
  {
    float offset_distance = parser.value_float();
    if (!WITHIN(offset_distance, 0.1, 5.0)) {
      SERIAL_ECHO_MSG("M3032 P<offset distance> out of range(0.1-0.5)");
      return;
    }
    move_away[0].x = -offset_distance;
    move_away[0].y = -offset_distance;
    move_away[1].x =  2.0f * offset_distance;
    move_away[1].y =  0.0f;
    move_away[2].x =  0.0f;
    move_away[2].y =  2.0f * offset_distance;
    move_away[3].x = -2.0f * offset_distance;
    move_away[3].y =  0.0f;
    SERIAL_ECHOLNPAIR(" move_away=({", move_away[0].x, ",", move_away[0].y, "},{", move_away[1].x, ",", move_away[1].y, "},{", move_away[2].x, ",", move_away[2].y,"},{", move_away[3].x, ",", move_away[3].y,"}");
  }
}


xy_pos_t M3032_Get_move_away(uint8_t position)
{
  if(position >= 4){
    position = 0;
  }
  return move_away[position];
}

#endif


void GcodeSuite::M3033()
{// M3033 [S<bedlevel_temp>] 
  
  if (parser.seenval('S'))
  {
    celsius_t temp = 0;
    temp = parser.value_celsius();
    if (!WITHIN(temp, 60, 250)) {
      SERIAL_ECHO_MSG("M3033 [S<bedlevel_temp>]  out of range(60-250)");
      return;
    }
    MYSERIAL2.printLine("M3033 [S<bedlevel temprature>] = %d\n", (int16_t)temp);
  }
}


void GcodeSuite::M3034() //
{// M3034 [C<Cancel alarm>] [S<Clogged nozzle switch>] [T<Clogged nozzle threshold>]
 
  // NOTE:-1 means invalid data. nozzle_switch = -1/0/1; threshold = -1/Other numbers besides -1;
  clogged_nozzle_t  clogged_nozzle;
  clogged_nozzle.type = DTF_NONE;
  clogged_nozzle.countclear = -1;
  clogged_nozzle.enable = -1;
  clogged_nozzle.threshold = -1;

  if (parser.seenval('C'))
  {
    clogged_nozzle.type = CANCEL_ALALRN;
    clogged_nozzle.countclear = parser.value_long();
    MYSERIAL2.printLine("echo: M3034 [C<Cancel alarm>] = %d\n", clogged_nozzle.countclear);
  }else{
    clogged_nozzle.type = PARM_MODIFICATION;
    if (parser.seenval('S'))
    {
      clogged_nozzle.enable = parser.value_int();
      MYSERIAL2.printLine("echo: M3034 [S<switch>] = %d\n", clogged_nozzle.enable);
    }

    if (parser.seenval('T'))
    {
      clogged_nozzle.threshold = parser.value_long();
      MYSERIAL2.printLine("echo: M3034 [T<threshold>] = %d\n", clogged_nozzle.threshold);
    }
  }

  if(clogged_nozzle.enable != -1 || clogged_nozzle.threshold != -1 ||  clogged_nozzle.type == CANCEL_ALALRN)
  {
    uart_nozzle_tx_clogged_nozzle(clogged_nozzle);
  }
}


#endif
