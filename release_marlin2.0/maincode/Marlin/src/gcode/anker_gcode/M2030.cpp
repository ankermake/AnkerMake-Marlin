/*
 * @Author       : tyrone
 * @Date         : 2022-08-16
 * @LastEditors  : tyrone
 * @LastEditTime : 2022-08-16
 * @Description  :
 */

#include "../../inc/MarlinConfig.h"
#include "../gcode.h"
#include "../../module/settings.h"

#define LIN_PARAM_LEN  12
static float lin_parameter[LIN_PARAM_LEN] = {0};

#if ENABLED(ANKER_LIN_PARAMETER)

/**
 * M2030: if doesn't see any character then exit ,others get the value after the character
 *
 *      A = ;
 *      B = ;
 *      C = ;
 */

void GcodeSuite::M2030()
{
    if (!parser.seen("ABCDEF"))
    {
        SERIAL_ECHOLN("M2030 need parameter");
        return ;
    }

    char *p = parser.command_ptr;
    int len = strlen(parser.command_ptr);

    if(len > 96){
        SERIAL_ECHOLN("M2030 order too long");
        return ;
    }

    for(int i = 0;i < len;i ++)
    {
        if(*p >= 'A' && *p <= 'Z')
        {   
            p++;
            float fl_result = strtof(p,nullptr);
            lin_parameter[*p - 'A'] = fl_result;
            SERIAL_ECHOPAIR("lin_parameter:",*p - 'A');
            SERIAL_ECHOPAIR_F(" - result:",fl_result);
            SERIAL_ECHO("\r\n");
        } 
        p++;
    }    
}

float GcodeSuite::M2031()
{
    int i = -1;
    if (!parser.seen("ABCDEF"))
    {
        SERIAL_ECHOLN("M2031 need parameter");
        return 0;
    }

    if (parser.seenval('A')) i = 0;
    if (parser.seenval('B')) i = 1;
    if (parser.seenval('C')) i = 2;
    if (parser.seenval('D')) i = 3;
    if (parser.seenval('E')) i = 4;
    if (parser.seenval('F')) i = 5;
    
    if(i < 0)
        return 0;
    else{
        SERIAL_ECHOLNPAIR("lin_parameter[i]:",lin_parameter[i]);
        return lin_parameter[i];
    }        
}

#endif

