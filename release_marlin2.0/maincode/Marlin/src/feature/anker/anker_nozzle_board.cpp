/*
 * @Author       : winter
 * @Date         : 2022-05-12 20:30:21
 * @LastEditors: winter.tian winter.tian@anker-in.com
 * @LastEditTime: 2022-09-30 18:01:29
 * @Description  :
 */
#include "anker_nozzle_board.h"
#include "../../gcode/gcode.h"
#include "../../module/planner.h"

#if ENABLED(ANKER_MAKE_API)

#if ENABLED(ANKER_NOZZLE_BOARD)

#define DEBUG_LOG_PRINTF
#ifdef DEBUG_LOG_PRINTF
#define debug_log_printf(format, args...) MYSERIAL2.printf(format, ##args)
#else
#define debug_log_printf(format, args...)
#endif

#define NOZZLE_SERIAL_READ() MYSERIAL3.read()
#define NOZZLE_SERIAL_AVAILABLE() MYSERIAL3.available()
#define NOZZLE_SERIAL_WRITE(format, args...) MYSERIAL3.printf(format, ##args)

#define FACTOR (0x107 & 0xFF)
static unsigned char crc8(unsigned char *pdat, unsigned int len)
{
    unsigned char j;
    unsigned char crc = 0x00;
    while (len--)
    {
        crc ^= (*pdat++);
        for (j = 8; j > 0; j--)
        {
            crc <<= 1;
            if (crc & 0x80)
            {
                crc ^= FACTOR;
            }
        }
    }

    return crc;
}

static void anker_nozzle_board_power_reset(void)
{
    anker_nozzle_board_info_t *p_info = get_anker_nozzle_board_info();

#ifdef NOZZLE_BOARD_PWR_PIN
    OUT_WRITE(NOZZLE_BOARD_PWR_PIN, !NOZZLE_BOARD_PWR_STATE);
    _delay_ms(100);
    OUT_WRITE(NOZZLE_BOARD_PWR_PIN, NOZZLE_BOARD_PWR_STATE);
    _delay_ms(10);
    p_info->reset_flag = 1;
    p_info->txrx_mode_flag = 1;
    p_info->serial_disable_state = 0;
    debug_log_printf("%s : reset OK\r\n", __func__);
#endif
}
static void anker_nozzle_board_serial_begin(void)
{
    anker_nozzle_board_info_t *p_info = get_anker_nozzle_board_info();

#ifdef SERIAL_PORT_3
#ifndef BAUDRATE_3
#define BAUDRATE_3 BAUDRATE
#endif
    p_info->serial_end();
    if (p_info->serial_state == ANKER_NOZZLE_BOARD_SERIAL_TXRX_CLOSE)
    {
        MYSERIAL3.begin(BAUDRATE_3);
        millis_t serial_connect_timeout = millis() + 1000UL;
        while (!MYSERIAL3.connected() && PENDING(millis(), serial_connect_timeout))
        {
        }
        p_info->serial_state = ANKER_NOZZLE_BOARD_SERIAL_TXRX_OPEN;
        debug_log_printf("%s : serial open\r\n", __func__);
    }
#endif
}
static void anker_nozzle_board_serial_end(void)
{
    anker_nozzle_board_info_t *p_info = get_anker_nozzle_board_info();

#ifdef SERIAL_PORT_3
    if (p_info->serial_state == ANKER_NOZZLE_BOARD_SERIAL_TXRX_OPEN)
    {
        MYSERIAL3.end();
        p_info->serial_state = ANKER_NOZZLE_BOARD_SERIAL_TXRX_CLOSE;
        debug_log_printf("%s : serial close\r\n", __func__);
        _delay_ms(10);
    }
#endif

#if ENABLED(PROVE_CONTROL)
    OUT_WRITE(PROVE_CONTROL_PIN, !PROVE_CONTROL_STATE);
    debug_log_printf("%s : tx output mode\r\n", __func__);
#endif
}

static int anker_nozzle_board_version_confirm(char *version)
{
    anker_nozzle_board_info_t *p_info = get_anker_nozzle_board_info();
    int major = 0, minor = 0, last = 0, m = 0, n = 0, x = 0;

    if (version == NULL)
    {
        debug_log_printf("%s : version is NULL !\r\n", __func__);
        return -1;
    }

    sscanf(p_info->software_version, "%d.%d.%d", &major, &minor, &last);
    sscanf(version, "%d.%d.%d", &m, &n, &x);
    // NOZZLE_SERIAL_WRITE(" p_info->software_version : %d - %d - %d\r\n", major, minor, last);
    // NOZZLE_SERIAL_WRITE(" compare_version : %d - %d - %d\r\n", m, n, x);
    // debug_log_printf("%s : cur %s - %s\r\n", __func__, p_info->software_version, version);

    return ((major > m) ? 1 : (minor > n)) ? 1 : (last >= x);
}
static void anker_nozzle_board_heartbeat_deal(void)
{
    anker_nozzle_board_info_t *p_info = get_anker_nozzle_board_info();
    static millis_t heartbeat_timeout = 0;
    static uint8_t pre_heartbeat_abnormal_flag = 0;
    static char tmp_str[16] = {0};

    if ((p_info->serial_disable_state == 0) && (anker_nozzle_board_version_confirm((char *)"2.0.5") == 1))
    {
        switch (p_info->heartbeat_deal_step)
        {
        case 0:
        {
            heartbeat_timeout = millis() + 10000UL;
            p_info->heartbeat_deal_step++;
            break;
        }
        case 1:
        {
            if (p_info->heartbeat_flag == 1)
            {
                p_info->heartbeat_flag = 0;
                p_info->heartbeat_abnormal_flag = 0;
                p_info->heartbeat_deal_step = 0;
            }
            else
            {
                if (!PENDING(millis(), heartbeat_timeout))
                {
                    p_info->heartbeat_abnormal_flag = 1;
                    p_info->heartbeat_deal_step = 0;
                }
            }

            if (pre_heartbeat_abnormal_flag != p_info->heartbeat_abnormal_flag)
            {
                pre_heartbeat_abnormal_flag = p_info->heartbeat_abnormal_flag;
                SERIAL_ECHOPAIR("echo:M3005,", p_info->heartbeat_abnormal_flag, "\r\n");
                if (p_info->heartbeat_abnormal_flag == 0)
                {
                    snprintf(tmp_str, sizeof(tmp_str), "M3003 V%d\n", p_info->threshold);
                    gcode.process_subcommands_now_P(tmp_str);
                    debug_log_printf("%s : excute M3003 cmd\r\n", __func__);
                }
            }
            break;
        }

        default:
        {
            p_info->heartbeat_deal_step = 0;
            break;
        }
        }
    }
    else
    {
        p_info->heartbeat_deal_step = 0;
    }
}
static uint8_t anker_nozzle_board_tx_cmd_ring_buf_available(void)
{
    anker_nozzle_board_info_t *p_info = get_anker_nozzle_board_info();
    return (uint8_t)(ANKER_NOZZLE_BOARD_TX_CMD_NUM + p_info->tx_cmd_ring_buf.w_index - p_info->tx_cmd_ring_buf.r_index) % ANKER_NOZZLE_BOARD_TX_CMD_NUM;
}
static int anker_nozzle_board_tx_cmd_ring_buf_read(char *buf, uint8_t size)
{
    anker_nozzle_board_info_t *p_info = get_anker_nozzle_board_info();
    anker_nozzle_board_tx_cmd_ring_buf_t *tmp_ring_buf = &p_info->tx_cmd_ring_buf;
    uint8_t tmp_len = 0;

    if (buf == NULL)
    {
        debug_log_printf("%s : buf is NULL !\r\n", __func__);
        return -1;
    }
    debug_log_printf("%s : r_index %d - w_index %d\r\n", __func__, tmp_ring_buf->r_index, tmp_ring_buf->w_index);
    if (tmp_ring_buf->r_index == tmp_ring_buf->w_index)
    {
        debug_log_printf("%s : tx_cmd_ring_buf is empty!\r\n", __func__);
        return -2;
    }
    else
    {
        tmp_len = strlen(tmp_ring_buf->buf[tmp_ring_buf->r_index]);
        if (tmp_len > size)
        {
            debug_log_printf("%s : tx_cmd_len > buf_size !\r\n", __func__);
            return -3;
        }
        else
        {
            strncpy(buf, tmp_ring_buf->buf[tmp_ring_buf->r_index], tmp_len);
            tmp_ring_buf->r_index = (uint8_t)(tmp_ring_buf->r_index + 1) % ANKER_NOZZLE_BOARD_TX_CMD_NUM;
            debug_log_printf("%s : %s - r_index %d\r\n", __func__, buf, tmp_ring_buf->r_index);
        }
    }

    return 0;
}
static int anker_nozzle_board_tx_cmd_ring_buf_write(char *buf, uint8_t len)
{
    anker_nozzle_board_info_t *p_info = get_anker_nozzle_board_info();
    anker_nozzle_board_tx_cmd_ring_buf_t *tmp_ring_buf = &p_info->tx_cmd_ring_buf;
    uint8_t tmp_w_index;

    if ((buf == NULL) || (len >= ANKER_NOZZLE_BOARD_TX_CMD_NUM))
    {
        debug_log_printf("%s : buf is NULL or len is too long !\r\n", __func__);
        return -1;
    }

    tmp_w_index = (uint8_t)(tmp_ring_buf->w_index + 1) % ANKER_NOZZLE_BOARD_TX_CMD_NUM;
    if (tmp_w_index == tmp_ring_buf->r_index)
    {
        debug_log_printf("%s : tx_cmd_ring_buf is full!\r\n", __func__);
        return -2;
    }
    else
    {
        memset(tmp_ring_buf->buf[tmp_ring_buf->w_index], 0, sizeof(tmp_ring_buf->buf[tmp_ring_buf->w_index]));
        strncpy(tmp_ring_buf->buf[tmp_ring_buf->w_index], buf, len);
        debug_log_printf("%s : %s - w_index %d\r\n", __func__, tmp_ring_buf->buf[tmp_ring_buf->w_index], tmp_ring_buf->w_index);
        tmp_ring_buf->w_index = tmp_w_index;
    }
    debug_log_printf("%s : w_index %d - r_index %d\r\n", __func__, tmp_ring_buf->w_index, tmp_ring_buf->r_index);

    return 0;
}
static int anker_nozzle_board_tx_cmd_ring_buf_add(char *cmd)
{
    char tmp_buf[ANKER_NOZZLE_BOARD_TX_BUF_SIZE] = {0};
    char tmp_crc_buf[8] = {0};
    uint8_t tmp_crc8 = 0;

    if (cmd == NULL)
    {
        debug_log_printf("%s : cmd is NULL !\r\n", __func__);
        return -1;
    }
    if (strlen(cmd) >= (ANKER_NOZZLE_BOARD_TX_BUF_SIZE - 6))
    {
        debug_log_printf("%s : cmd is too long !\r\n", __func__);
        return -2;
    }
    snprintf(tmp_buf, sizeof(tmp_buf), "%s ", cmd);
    tmp_crc8 = crc8((unsigned char *)tmp_buf, strlen(tmp_buf));
    sprintf(tmp_crc_buf, "*%d\n", tmp_crc8);
    strcat(tmp_buf, tmp_crc_buf);
    if (anker_nozzle_board_tx_cmd_ring_buf_write(tmp_buf, strlen(tmp_buf)) != 0)
    {
        debug_log_printf("%s : cmd add failed !\r\n", __func__);
        return -3;
    }

    return 0;
}
static void anker_nozzle_board_tx_deal(void)
{
    anker_nozzle_board_info_t *p_info = get_anker_nozzle_board_info();
    char tmp_buf[ANKER_NOZZLE_BOARD_TX_BUF_SIZE] = {0};
    static uint8_t tmp_com_abnormal_pre_flag = 0;
    static millis_t tmp_timeout = 0;
    static uint8_t tmp_tx_retry_count = 0;

    if (tmp_com_abnormal_pre_flag != p_info->com_abnormal_flag)
    {
        tmp_com_abnormal_pre_flag = p_info->com_abnormal_flag;
        // SERIAL_ECHOPAIR("echo:M3006,", p_info->com_abnormal_flag, "\r\n");
    }

    switch (p_info->tx_deal_step)
    {
    case 0:
    {
        debug_log_printf("%s : tx_deal_step %d\r\n", __func__, p_info->tx_deal_step);
        if (p_info->tx_init_flag == 0)
        {
            memset(tmp_buf, 0, sizeof(tmp_buf));
            sprintf(tmp_buf, "M3003 %d", p_info->threshold);
            if (p_info->tx_ring_buf_add(tmp_buf) == 0)
            {
                p_info->power_reset();
                p_info->tx_deal_step++;
                p_info->tx_deal_step = 3;
            }
        }
        else
        {
            p_info->tx_deal_step = 10;
        }
        break;
    }
    case 1:
    {
        memset(tmp_buf, 0, sizeof(tmp_buf));
        sprintf(tmp_buf, "M3008 %d", p_info->fireproof_adc0);
        if (p_info->tx_ring_buf_add(tmp_buf) == 0)
        {
            p_info->tx_deal_step++;
        }
        break;
    }
    case 2:
    {
        memset(tmp_buf, 0, sizeof(tmp_buf));
        sprintf(tmp_buf, "M3009 %d", p_info->fireproof_adc1);
        if (p_info->tx_ring_buf_add(tmp_buf) == 0)
        {
            p_info->tx_deal_step++;
        }
        break;
    }
    case 3:
    {
        memset(tmp_buf, 0, sizeof(tmp_buf));
        sprintf(tmp_buf, "M3010");
        if (p_info->tx_ring_buf_add(tmp_buf) == 0)
        {
            p_info->tx_init_flag = 1;
            p_info->tx_deal_step = 10;
        }
        break;
    }
    case 10:
    {
        if ((p_info->serial_disable_state == 0) && (p_info->serial_state == ANKER_NOZZLE_BOARD_SERIAL_TXRX_CLOSE))
        {
            p_info->serial_begin();
            debug_log_printf("%s : tx_deal_step %d, serial_start\r\n", __func__, p_info->tx_deal_step);
            if (p_info->txrx_mode_flag == 0)
            {
#if ENABLED(PROVE_CONTROL)
                OUT_WRITE(PROVE_CONTROL_PIN, !PROVE_CONTROL_STATE);
#endif
                debug_log_printf("%s : tx_deal_step %d - tx output mode\r\n", __func__, p_info->tx_deal_step);
            }
        }
        else if ((p_info->serial_disable_state == 1) && (p_info->serial_state == ANKER_NOZZLE_BOARD_SERIAL_TXRX_OPEN))
        {
            p_info->serial_end();
            debug_log_printf("%s : tx_deal_step %d - serial_end\r\n", __func__, p_info->tx_deal_step);
        }

        if ((p_info->serial_state == ANKER_NOZZLE_BOARD_SERIAL_TXRX_OPEN) && (p_info->tx_ring_buf_available() > 0))
        {
            debug_log_printf("%s : tx_deal_step %d - ring_buf_read\r\n", __func__, p_info->tx_deal_step);
            if (p_info->tx_ring_buf_read(p_info->tx_buf, sizeof(p_info->tx_buf)) == 0)
            {
                tmp_tx_retry_count = 0;
                p_info->tx_deal_step++;
            }
        }
        break;
    }
    case 11:
    {
        debug_log_printf("%s : tx_deal_step %d\r\n", __func__, p_info->tx_deal_step);
        tmp_timeout = millis() + 100UL;
        p_info->tx_deal_step++;
        break;
    }
    case 12:
    {
        if (!PENDING(millis(), tmp_timeout))
        {
#ifdef SERIAL_PORT_3
            NOZZLE_SERIAL_WRITE("%s\n", p_info->tx_buf);
#endif
            debug_log_printf("%s : tx_deal_step %d - TX %s\r\n", __func__, p_info->tx_deal_step, p_info->tx_buf);
            tmp_timeout = millis() + 2000UL;
            p_info->tx_state = TX_START_STATE;
            p_info->tx_deal_step++;
        }
        break;
    }
    case 13:
    {
        if (p_info->tx_state == TX_SUCCESS_STATE)
        {
            if (strstr(p_info->tx_buf, "M3010") != NULL)
            {
                p_info->serial_end();
                p_info->txrx_mode_flag = 0;
                debug_log_printf("%s : tx_deal_step %d - M3010 serial close\r\n", __func__, p_info->tx_deal_step);
            }
            memset(p_info->tx_buf, 0, sizeof(p_info->tx_buf));
            p_info->com_abnormal_flag = 0;
            p_info->tx_deal_step = 10;
        }
        else
        {
            if (!PENDING(millis(), tmp_timeout))
            {
                NOZZLE_SERIAL_WRITE(" TX_TOUT_STATE\r\n");
                debug_log_printf("%s : tx_deal_step %d - TX_TOUT_STATE\r\n", __func__, p_info->tx_deal_step);
                p_info->tx_state = TX_TIMEOUT_STATE;
                p_info->tx_deal_step++;
            }
        }
        break;
    }
    case 14:
    {
        tmp_tx_retry_count++;
        if (tmp_tx_retry_count >= 3)
        {
            NOZZLE_SERIAL_WRITE(" TX_ERROR_STATE\r\n");
            debug_log_printf("%s : tx_deal_step %d - TX_ERROR_STATE\r\n", __func__, p_info->tx_deal_step);
            p_info->tx_state = TX_ERROR_STATE;
            p_info->tx_error_times++;
            p_info->com_abnormal_flag = 1;
            memset(p_info->tx_buf, 0, sizeof(p_info->tx_buf));
            p_info->tx_deal_step = 10;
        }
        else
        {
            p_info->tx_deal_step = 11;
        }
        break;
    }

    default:
    {
        p_info->tx_deal_step = 0;
        break;
    }
    }
}

static int anker_nozzle_board_get_software_version(anker_nozzle_board_cmd_params_t *params)
{
    anker_nozzle_board_info_t *p_info = get_anker_nozzle_board_info();
    char *p = NULL;

    if(params == NULL)
    {
        debug_log_printf("%s : params is NULL\r\n", __func__);
        return -1;
    }

    if ((p = strstr(params->cmd, "software_version")) == NULL)
    {
        return -2;
    }
    sscanf(p, "software_version: %s", p_info->software_version);
    debug_log_printf("%s : %s\r\n", __func__, p);
    // NOZZLE_SERIAL_WRITE(" p_info->software_version : %s\r\n", p_info->software_version);

    if (p_info->reset_flag == 0)
    {
        p_info->abnormal_reset_times++;
    }

    return 0;
}
static int anker_nozzle_board_heartbeat(anker_nozzle_board_cmd_params_t *params)
{
    anker_nozzle_board_info_t *p_info = get_anker_nozzle_board_info();
    char *p = NULL;

    if(params == NULL)
    {
        debug_log_printf("%s : params is NULL\r\n", __func__);
        return -1;
    }

    if ((p = strstr(params->cmd, "heartbeat")) == NULL)
        return -2;
    p_info->heartbeat_flag = 1;
    if (anker_nozzle_board_version_confirm((char *)"2.0.6") == 1)
    {
        sscanf(p, "heartbeat %s", p_info->software_version);
    }
    // debug_log_printf("%s : rx_cmd <- %s\r\n", __func__, p);

    return 0;
}
static int anker_nozzle_board_get_threshold_init_value(anker_nozzle_board_cmd_params_t *params)
{
    anker_nozzle_board_info_t *p_info = get_anker_nozzle_board_info();
    char *p = NULL;

    if(params == NULL)
    {
        debug_log_printf("%s : params is NULL\r\n", __func__);
        return -1;
    }

    if ((p = strstr(params->cmd, "M3001")) == NULL)
    {
        return -2;
    }
    sscanf(p, "M3001 %d", &(p_info->threshold_init_value));
    SERIAL_ECHOPAIR("echo:M3001 V", p_info->threshold_init_value, "\r\n");
    return 0;
}
static int anker_nozzle_board_get_threshold_cur_value(anker_nozzle_board_cmd_params_t *params)
{
    anker_nozzle_board_info_t *p_info = get_anker_nozzle_board_info();
    char *p = NULL;

    if(params == NULL)
    {
        debug_log_printf("%s : params is NULL\r\n", __func__);
        return -1;
    }

    if ((p = strstr(params->cmd, "M3002")) == NULL)
    {
        return -2;
    }
    sscanf(p, "M3002 %d", &(p_info->threshold_cur_value));
    SERIAL_ECHOPAIR("echo:M3002 V", p_info->threshold_cur_value, "\r\n");
    return 0;
}
static int anker_nozzle_board_set_threshold_value(anker_nozzle_board_cmd_params_t *params)
{
    char *p = NULL;

    if(params == NULL)
    {
        debug_log_printf("%s : params is NULL\r\n", __func__);
        return -1;
    }

    if ((p = strstr(params->cmd, "M3003 OK")) == NULL)
    {
        return -2;
    }
    debug_log_printf("%s : set_threshold_value success\r\n", __func__);
    return 0;
}
static int anker_nozzle_board_get_fan_info(anker_nozzle_board_cmd_params_t *params)
{
    char *p = NULL;

    if(params == NULL)
    {
        debug_log_printf("%s : params is NULL\r\n", __func__);
        return -1;
    }

    if ((p = strstr(params->cmd, "M3004")) == NULL)
    {
        return -2;
    }
    debug_log_printf("%s : %s\r\n", __func__, p);
    return 0;
}
static int anker_nozzle_board_set_fan_pwm_num(anker_nozzle_board_cmd_params_t *params)
{
    char *p = NULL;

    if(params == NULL)
    {
        debug_log_printf("%s : params is NULL\r\n", __func__);
        return -1;
    }

    if ((p = strstr(params->cmd, "M3005 OK")) == NULL)
    {
        return -2;
    }
    debug_log_printf("%s : set_fan_pwm_num success\r\n", __func__);
    return 0;
}
static int anker_nozzle_board_set_fan_pwm(anker_nozzle_board_cmd_params_t *params)
{
    char *p = NULL;

    if(params == NULL)
    {
        debug_log_printf("%s : params is NULL\r\n", __func__);
        return -1;
    }

    if ((p = strstr(params->cmd, "M3006 OK")) == NULL)
    {
        return -2;
    }
    debug_log_printf("%s : set_fan_pwm success\r\n", __func__);
    return 0;
}
static int anker_nozzle_board_get_fireproof_info(anker_nozzle_board_cmd_params_t *params)
{
    char *p = NULL;

    if(params == NULL)
    {
        debug_log_printf("%s : params is NULL\r\n", __func__);
        return -1;
    }

    if ((p = strstr(params->cmd, "M3007")) == NULL)
    {
        return -2;
    }
    debug_log_printf("%s : %s\r\n", __func__, p);
    return 0;
}
static int anker_nozzle_board_set_fireproof1(anker_nozzle_board_cmd_params_t *params)
{
    char *p = NULL;

    if(params == NULL)
    {
        debug_log_printf("%s : params is NULL\r\n", __func__);
        return -1;
    }

    if ((p = strstr(params->cmd, "M3008 OK")) == NULL)
    {
        return -2;
    }
    debug_log_printf("%s : set_fireproof1 success\r\n", __func__);
    return 0;
}
static int anker_nozzle_board_set_fireproof2(anker_nozzle_board_cmd_params_t *params)
{
    char *p = NULL;

    if(params == NULL)
    {
        debug_log_printf("%s : params is NULL\r\n", __func__);
        return -1;
    }

    if ((p = strstr(params->cmd, "M3009 OK")) == NULL)
    {
        return -2;
    }
    debug_log_printf("%s : set_fireproof2 success\r\n", __func__);
    return 0;
}
static int anker_nozzle_board_quit_txrx_mode(anker_nozzle_board_cmd_params_t *params)
{
    char *p = NULL;

    if(params == NULL)
    {
        debug_log_printf("%s : params is NULL\r\n", __func__);
        return -1;
    }

    if ((p = strstr(params->cmd, "M3010 OK")) == NULL)
    {
        return -2;
    }
    debug_log_printf("%s : quit_txrx_mode\r\n", __func__);
    return 0;
}

anker_nozzle_board_rx_cmd_array_t anker_nozzle_board_rx_cmd_array[] = {
    {(char *)"software_version", anker_nozzle_board_get_software_version, "get_software_version"},
    {(char *)"heartbeat", anker_nozzle_board_heartbeat, "heartbeat"},
    {(char *)"M3001", anker_nozzle_board_get_threshold_init_value, "get_threshold_init_value"}, // M3001
    {(char *)"M3002", anker_nozzle_board_get_threshold_cur_value, "get_threshold_cur_value"},   // M3002
    {(char *)"M3003", anker_nozzle_board_set_threshold_value, "set_threshold_value"},           // M3003 value
    {(char *)"M3004", anker_nozzle_board_get_fan_info, "get_fan_info"},                         // M3004
    {(char *)"M3005", anker_nozzle_board_set_fan_pwm_num, "set_fan_pwm_num"},                   // M3005 value
    {(char *)"M3006", anker_nozzle_board_set_fan_pwm, "set_fan_pwm"},                           // M3006 num adc pwm
    {(char *)"M3007", anker_nozzle_board_get_fireproof_info, "get_fireproof_info"},             // M3007
    {(char *)"M3008", anker_nozzle_board_set_fireproof1, "set_fireproof1"},                     // M3008 value
    {(char *)"M3009", anker_nozzle_board_set_fireproof2, "set_fireproof2"},                     // M3009 value
    {(char *)"M3010", anker_nozzle_board_quit_txrx_mode, "quit_txrx_mode"},                     // M3010
};

static void anker_nozzle_board_find_cmd_handler(anker_nozzle_board_cmd_params_t *params)
{
    anker_nozzle_board_info_t *p_info = get_anker_nozzle_board_info();
    char *p = NULL;

    if(params == NULL)
    {
        debug_log_printf("%s : params is NULL\r\n", __func__);
        return;
    }

    for (uint8_t i = 0; i < sizeof(anker_nozzle_board_rx_cmd_array) / sizeof(anker_nozzle_board_rx_cmd_array_t); i++)
    {
        p = strstr(params->cmd, anker_nozzle_board_rx_cmd_array[i].keywords);
        if (p != NULL)
        {
            params->keywords = anker_nozzle_board_rx_cmd_array[i].keywords;
            if (anker_nozzle_board_rx_cmd_array[i].cmd_handler(params) != 0)
            {
                debug_log_printf("%s : %s failed\r\n", __func__, anker_nozzle_board_rx_cmd_array[i].brief);
            }
            else
            {
                if (strstr(p_info->tx_buf, params->keywords) != NULL)
                {
                    p_info->tx_state = TX_SUCCESS_STATE;
                    debug_log_printf("%s : TX_SUCCESS_STATE\r\n", __func__);
                }
            }
        }
    }
}
static void anker_nozzle_board_rx_deal(void)
{
    anker_nozzle_board_info_t *p_info = get_anker_nozzle_board_info();

#ifdef SERIAL_PORT_3
    if ((p_info->serial_state == ANKER_NOZZLE_BOARD_SERIAL_TXRX_OPEN) && (NOZZLE_SERIAL_AVAILABLE() > 0))
    {
        const int c = NOZZLE_SERIAL_READ();
        const char ch = (char)c;

        if (ch > 0)
        {
            if ((p_info->rx_info.count > 0) && (ISEOL(ch)))
            {
                // debug_log_printf("%s : rx_cmd <- %s\r\n", __func__, p_info->rx_info.buf);
                // NOZZLE_SERIAL_WRITE(" rx_cmd <- count : %d\r\n", p_info->rx_info.count);
                // NOZZLE_SERIAL_WRITE(" rx_cmd <- buf : %s\r\n", p_info->rx_info.buf);
                p_info->cmd_params.cmd = p_info->rx_info.buf;
                p_info->find_cmd_handler(&p_info->cmd_params);
                memset(&p_info->rx_info, 0, sizeof(p_info->rx_info));
            }
            else
            {
                if (ISEOL(ch))
                    return;
                p_info->rx_info.buf[p_info->rx_info.count++] = ch;
                if (p_info->rx_info.count >= ANKER_NOZZLE_BOARD_RX_BUF_SIZE)
                {
                    debug_log_printf("%s : rx command too long ...[>RX_BUF_SIZE]...\r\n", __func__);
                    memset(&p_info->rx_info, 0, sizeof(p_info->rx_info));
                }
            }
        }
    }
#endif
}
static void anker_nozzle_board_deal(void)
{
    anker_nozzle_board_info_t *p_info = get_anker_nozzle_board_info();

    p_info->rx_deal();
    p_info->tx_deal();
    p_info->heartbeat_deal();
    watchdog_refresh();
}

anker_nozzle_board_info_t *get_anker_nozzle_board_info(void)
{
    static anker_nozzle_board_info_t anker_nozzle_board_info = {0};
    return &anker_nozzle_board_info;
}

void anker_nozzle_board_init(void)
{
    anker_nozzle_board_info_t *p_info = get_anker_nozzle_board_info();

    p_info->power_reset = anker_nozzle_board_power_reset;
    p_info->serial_begin = anker_nozzle_board_serial_begin;
    p_info->serial_end = anker_nozzle_board_serial_end;
    p_info->tx_ring_buf_available = anker_nozzle_board_tx_cmd_ring_buf_available;
    p_info->tx_ring_buf_read = anker_nozzle_board_tx_cmd_ring_buf_read;
    p_info->tx_ring_buf_write = anker_nozzle_board_tx_cmd_ring_buf_write;
    p_info->tx_ring_buf_add = anker_nozzle_board_tx_cmd_ring_buf_add;
    p_info->tx_deal = anker_nozzle_board_tx_deal;
    p_info->rx_deal = anker_nozzle_board_rx_deal;
    p_info->find_cmd_handler = anker_nozzle_board_find_cmd_handler;
    p_info->heartbeat_deal = anker_nozzle_board_heartbeat_deal;
    p_info->nozzle_board_deal = anker_nozzle_board_deal;

    debug_log_printf("%s\r\n", __func__);
    p_info->serial_begin();
}

void set_anker_z_sensorless_probe_value(int value)
{
    char str[100] = "";
    get_anker_nozzle_board_info()->is_z_sensorless = true;
    get_anker_nozzle_board_info()->z_sensorless_threshols = get_anker_nozzle_board_info()->threshold;
    get_anker_nozzle_board_info()->threshold = value;
    sprintf(str, "M3003 V%d\n", get_anker_nozzle_board_info()->threshold);
    gcode.process_subcommands_now_P(str);
}

void reset_anker_z_sensorless_probe_value()
{
    char str[100] = "";
    get_anker_nozzle_board_info()->is_z_sensorless = false;
    get_anker_nozzle_board_info()->threshold = get_anker_nozzle_board_info()->z_sensorless_threshols;
    sprintf(str, "M3003 V%d\n", get_anker_nozzle_board_info()->threshold);
    gcode.process_subcommands_now_P(str);
}

#endif /* ANKER_NOZZLE_BOARD_THRESHOLD */

#endif /* ANKER_MAKE_API */
