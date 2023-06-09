/*
 * @Author: winter.tian
 * @Date: 2023-01-17 16:09:46
 * @LastEditors: winter.tian
 * @LastEditTime: 2023-03-24 17:54:06
 * @Description: 
 */
#include "../gcode.h"
#include "../../inc/MarlinConfig.h"
#include "../queue.h"           // for getting the command port

#if ENABLED(ANKER_MAKE_API)
  #include "../../feature/anker/board_configure.h"
//M116 M116 S0 Check the firmware version number
//M116 S1 Check the hardware version number
//M116 S2 Check the model of the main control chip
void GcodeSuite::M116() 
{
  if (parser.seen('S'))
  {
    switch (parser.value_int())
    {
    case 0:
      SERIAL_ECHOLNPGM(DETAILED_BUILD_VERSION);
      break;
    #if ENABLED(BOARD_CONFIGURE)
      case 1:
        SERIAL_ECHO(board_configure.board_version);
        SERIAL_ECHO("\r\n");
        break;
      case 2:
        SERIAL_ECHO(board_configure.board_chip);
        SERIAL_ECHO("\r\n");
        break;
    #endif
    default:
      break;
    }
  }
  else
  {
    char tmpbuf[64] = {0};
    snprintf(tmpbuf, sizeof(tmpbuf), "%s,%s", DETAILED_BUILD_VERSION, board_configure.board_version);
    SERIAL_ECHOLN(tmpbuf);
  }
}
#endif
