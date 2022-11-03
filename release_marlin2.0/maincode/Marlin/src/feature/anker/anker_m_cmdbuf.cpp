/*
 * @Author       : winter
 * @Date         : 2022-05-13 14:28:04
 * @LastEditors  : winter
 * @LastEditTime : 2022-07-08 12:09:41
 * @Description  :
 */
#include "anker_m_cmdbuf.h"
#include "../../gcode/queue.h"
#include "../../gcode/gcode.h"
#include "../../gcode/parser.h"

#if ENABLED(ANKER_MAKE_API)

#if ENABLED(ANKER_M_CMDBUF)

static void anker_m_cmdbuf_translate_xyze_axis(char *buf)
{
    char *token = NULL;

    token = strchr(buf, 'A');
    if(token!=NULL)
    {
        *token = 'X';
    }

    token = strchr(buf, 'B');
    if(token!=NULL)
    {
        *token = 'Y';
    }

    token = strchr(buf, 'C');
    if(token!=NULL)
    {
        *token = 'Z';
    }

    token = strchr(buf, 'D');
    if(token!=NULL)
    {
        *token = 'E';
    }
}

void anker_m_cmdbuf_init(void)
{
    anker_m_cmdbuf_info_t *p_info = get_anker_m_cmdbuf_info();

    p_info->queue_translate_xyze_axis = anker_m_cmdbuf_translate_xyze_axis;
}

anker_m_cmdbuf_info_t *get_anker_m_cmdbuf_info(void)
{
    static anker_m_cmdbuf_info_t anker_m_cmdbuf_info = {0};
    return &anker_m_cmdbuf_info;
}

#endif /* ENABLED(ANKER_M_CMDBUF) */

#endif /* ENABLED(ANKER_MAKE_API) */
