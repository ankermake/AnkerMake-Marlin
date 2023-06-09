#ifndef __GCP_PARSER_H__
#define __GCP_PARSER_H__

#include <stdint.h>
#include <stdbool.h>

#include "gcp.h"

typedef struct
{
    enum
    {
        Parse_Magic0,
        Parse_Magic1,
        Parse_Content_Len_L,
        Parse_Content_Len_H,
        Parse_Type_Version,
        Parse_Check,
        Parse_Src_Dst_Module,
        Parse_Cmd,
        Parse_Content
    } state;

    bool parse_en;

    uint32_t time_start_ms;
    uint16_t max_interval_ms;

    gcp_msg_t *msg;

    uint16_t cnt; 
    uint8_t sum;

    uint32_t (*get_time_ms)(void);
    void (*callback)(gcp_msg_t *);
} gcp_parser_t;

void gcp_parser_init(gcp_parser_t *gcp_parser,
                     uint8_t *buf,
                     uint16_t max_interval_ms,
                     uint32_t (*get_time_ms)(void),
                     void (*callback)(gcp_msg_t *));

void gcp_parser_enable(gcp_parser_t *gcp_parser);

void gcp_parser_disable(gcp_parser_t *gcp_parser);

void gcp_parser_fsm_process(gcp_parser_t *gcp_parser, uint8_t dat);

#endif // __GCP_PARSER_H__
