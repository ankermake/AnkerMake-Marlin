#include "gcp.h"

uint8_t gcp_calc_check(gcp_msg_t *msg)
{
    uint16_t i;
    uint8_t sum = 0;
    uint8_t *p = (uint8_t *)msg;

    sum += p[0];
    sum += p[1];
    sum += p[2];
    sum += p[3];
    sum += p[4];
    sum += p[6];
    sum += p[7];

    for (i = 0; i < msg->content_len; i++)
        sum += msg->content[i];

    return sum;
}

int gcp_msg_check(gcp_msg_t *msg)
{
    if (msg->magic0 != GCP_MAGIC0)
        return -1;

    if (msg->magic1 != GCP_MAGIC1)
        return -2;

    if (msg->version != GCP_VERSION)
        return -3;

    if (msg->check != gcp_calc_check(msg))
        return -4;

    return 0;
}

uint16_t gcp_msg_pack(gcp_msg_t *gcp_msg,
                     uint8_t type,
                     uint8_t src_module,
                     uint8_t dst_module,
                     uint8_t cmd,
                     uint8_t *content,
                     uint16_t content_len)
{
    uint16_t i;

    gcp_msg->magic0 = GCP_MAGIC0;
    gcp_msg->magic1 = GCP_MAGIC1;
    gcp_msg->content_len = content_len;
    gcp_msg->type = type;
    gcp_msg->version = GCP_VERSION;
    gcp_msg->src_module = src_module;
    gcp_msg->dst_module = dst_module;
    gcp_msg->cmd = cmd;

    for (i = 0; i < content_len; i++)
    {
        gcp_msg->content[i] = content[i];
    }

    gcp_msg->check = gcp_calc_check(gcp_msg);

    return (gcp_msg->content_len + 8);
}
