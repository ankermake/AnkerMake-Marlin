#ifndef __GCP_H__
#define __GCP_H__

#include <stdint.h>

#define GCP_MAGIC0 0xAA
#define GCP_MAGIC1 0x55

#define GCP_CONTENT_MAX_LEN 1024

#define GCP_TYPE_QUERY  0x01
#define GCP_TYPE_SET    0x02
#define GCP_TYPE_NOTIFY 0x03
#define GCP_TYPE_CMD    0x04
#define GCP_TYPE_ACK    0x05

#define GCP_VERSION 0

#define GCP_MSG_LEN(msg) (msg->content_len + 8)

typedef struct
{
    uint8_t magic0;
    uint8_t magic1;
    uint16_t content_len;

    uint8_t type : 3;
    uint8_t version : 5;
    uint8_t check;
    uint8_t src_module : 4;
    uint8_t dst_module : 4;
    uint8_t cmd;

    uint8_t content[GCP_CONTENT_MAX_LEN];
} gcp_msg_t;

#define GCP_CMD_00_CONNECT       0x00
#define GCP_CMD_01_BOOT_START    0x01
#define GCP_CMD_02_BOOT_FW_BLOCK 0x02
#define GCP_CMD_03_BOOT_END      0x03

uint8_t gcp_calc_check(gcp_msg_t *msg);

int gcp_msg_check(gcp_msg_t *msg);

uint16_t gcp_msg_pack(gcp_msg_t *gcp_msg,
                      uint8_t type,
                      uint8_t src_module,
                      uint8_t dst_module,
                      uint8_t cmd,
                      uint8_t *content,
                      uint16_t content_len);

#endif // __GCP_H__
