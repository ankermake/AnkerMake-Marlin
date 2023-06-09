#include "gcp_parser.h"

void gcp_parser_init(gcp_parser_t *gcp_parser,
                     uint8_t *buf,
                     uint16_t max_interval_ms,
                     uint32_t (*get_time_ms)(void),
                     void (*callback)(gcp_msg_t *))
{
    gcp_parser->msg = (gcp_msg_t *)buf;
    gcp_parser->max_interval_ms = max_interval_ms;
    gcp_parser->get_time_ms = get_time_ms;
    gcp_parser->callback = callback;

    gcp_parser->state = Parse_Magic0;
    gcp_parser->time_start_ms = 0;
    gcp_parser->cnt = 0;
    gcp_parser->sum = 0;
    gcp_parser->parse_en = true;
}

void gcp_parser_enable(gcp_parser_t *gcp_parser)
{
    gcp_parser->parse_en = true;
}

void gcp_parser_disable(gcp_parser_t *gcp_parser)
{
    gcp_parser->state = Parse_Magic0;
    gcp_parser->parse_en = false;
}

void gcp_parser_fsm_process(gcp_parser_t *gcp_parser, uint8_t dat)
{
    if (gcp_parser->parse_en == true)
    {
        if (gcp_parser->get_time_ms() - gcp_parser->time_start_ms > gcp_parser->max_interval_ms)
        {
            gcp_parser->state = Parse_Magic0;
        }

        switch (gcp_parser->state)
        {
        case Parse_Magic0:
            if (dat == GCP_MAGIC0)
            {
                gcp_parser->msg->magic0 = dat;
                gcp_parser->sum = 0;
                gcp_parser->sum += dat;
                gcp_parser->time_start_ms = gcp_parser->get_time_ms();
                gcp_parser->state = Parse_Magic1;
            }
            break;

        case Parse_Magic1:
            if (dat == GCP_MAGIC1)
            {
                gcp_parser->msg->magic1 = dat;
                gcp_parser->sum += dat;
                gcp_parser->time_start_ms = gcp_parser->get_time_ms();
                gcp_parser->state = Parse_Content_Len_L;
            }
            else
            {
                gcp_parser->state = Parse_Magic0;
            }
            break;

        case Parse_Content_Len_L:
            gcp_parser->msg->content_len = dat;
            gcp_parser->sum += dat;
            gcp_parser->time_start_ms = gcp_parser->get_time_ms();
            gcp_parser->state = Parse_Content_Len_H;
            break;

        case Parse_Content_Len_H:
            gcp_parser->msg->content_len |= dat << 8;
            gcp_parser->sum += dat;
            gcp_parser->time_start_ms = gcp_parser->get_time_ms();
            gcp_parser->state = Parse_Type_Version;
            break;

        case Parse_Type_Version:
            gcp_parser->msg->type = dat & 0x07;
            gcp_parser->msg->version = (dat >> 3) & 0x1F;
            gcp_parser->sum += dat;
            gcp_parser->time_start_ms = gcp_parser->get_time_ms();
            gcp_parser->state = Parse_Check;
            break;

        case Parse_Check:
            gcp_parser->msg->check = dat;
            gcp_parser->time_start_ms = gcp_parser->get_time_ms();
            gcp_parser->state = Parse_Src_Dst_Module;
            break;

        case Parse_Src_Dst_Module:
            gcp_parser->msg->src_module = dat & 0x0F;
            gcp_parser->msg->dst_module = (dat >> 4) & 0x0F;
            gcp_parser->sum += dat;
            gcp_parser->time_start_ms = gcp_parser->get_time_ms();
            gcp_parser->state = Parse_Cmd;
            break;

        case Parse_Cmd:
            gcp_parser->msg->cmd = dat;
            gcp_parser->sum += dat;
            if (gcp_parser->msg->content_len == 0 && gcp_parser->msg->check == gcp_parser->sum)
            {
                if (gcp_parser->callback != 0)
                    gcp_parser->callback(gcp_parser->msg);
                gcp_parser->time_start_ms = gcp_parser->get_time_ms();
                gcp_parser->state = Parse_Magic0;
            }
            else
            {
                gcp_parser->cnt = 0;
                gcp_parser->time_start_ms = gcp_parser->get_time_ms();
                gcp_parser->state = Parse_Content;
            }
            break;

        case Parse_Content:
            gcp_parser->msg->content[gcp_parser->cnt++] = dat;
            gcp_parser->sum += dat;
            if (gcp_parser->cnt == gcp_parser->msg->content_len)
            {
                if (gcp_parser->msg->check == gcp_parser->sum)
                {
                    if (gcp_parser->callback != 0)
                        gcp_parser->callback(gcp_parser->msg);
                }
                gcp_parser->time_start_ms = gcp_parser->get_time_ms();
                gcp_parser->state = Parse_Magic0;
            }
            break;

        default:
            gcp_parser->state = Parse_Magic0;
            break;
        }
    }
}
