/*
 * @Author       : winter
 * @Date         : 2022-04-02 10:50:38
 * @LastEditors: winter.tian
 * @LastEditTime: 2023-01-10 18:07:31
 * @Description  :
 */
#include "../../inc/MarlinConfig.h"
// #include "../../"
#include "../../gcode/queue.h"

#if ENABLED(ANKER_MAKE_API)

#if ENABLED(ANKER_PAUSE_FUNC)

#ifndef __ANKER_PAUSE_H__
#define __ANKER_PAUSE_H__

#define ANKER_PAUSE_CMD_STR "M2022"
#define ANKER_CONTINUE_CMD_STR "M2023"
#define ANKER_STOP_CMD_STR "M2024"
#define ANKER_CLEAR_STOP_CMD_STR "M2025"

enum anker_pause_status
{
    ANKER_PAUSE_IDLE,
    ANKER_PAUSE_START,
    ANKER_PAUSE_OK,
    ANKER_PAUSE_CONTINUE,
    ANKER_PAUSE_END,
    ANKER_CONTINUE_PRINT,
    ANKER_PAUSE_SERIAL_ENABLE,
    ANKER_PAUSE_SERIAL_OK,
    ANKER_PAUSE_SERIAL_DISABLE,
    ANKER_PAUSE_QUEUE_ENABLE,
    ANKER_PAUSE_QUEUE_OK,
    ANKER_PAUSE_QUEUE_DISABLE,
    ANKER_PAUSE_BLOCK_ENABLE,
    ANKER_PAUSE_BLOCK_OK,
    ANKER_PAUSE_BLOCK_DISABLE,
};

enum anker_pause_cmd_state
{
    ANKER_PAUSE_CMD_IDLE,
    ANKER_PAUSE_CMD_START,
    ANKER_PAUSE_CMD_CONTINUE,
};

typedef struct
{
    char buffer[MAX_CMD_SIZE];
    bool skip_ok;
#if HAS_MULTI_SERIAL
    serial_index_t port;
#endif
} anker_cmd_line_t;

typedef struct
{
    uint16_t length;
    uint16_t index_r;
    uint16_t index_w;
    anker_cmd_line_t commands[BUFSIZE];
} anker_queue_buffer_t;

typedef struct
{
    xyze_pos_t xyze_pos;
    float feed_rate_mm_s;
    uint8_t extruder;
    float millimeters;
} anker_block_info_t;

typedef struct
{
    xyze_pos_t cur_pos;
    float cur_fr_mm_s;
    uint8_t block_length;
    uint8_t block_info_head;
    uint8_t block_info_tail;
    anker_block_info_t block_info[BLOCK_BUFFER_SIZE];
} anker_block_buffer_t;

typedef struct
{
    int pause_flag;
    int pause_state;
    int pause_cmd_state;
    int pause_serial_state;
    int pause_queue_state;
    int pause_block_state;
    int pause_deal_step;
    char tmp_cmd_buf[256];
    uint8_t tmp_block_tail;
    uint8_t tmp_block_length;

    void (*pause_start)(void);
    void (*pause_continue)(void);
    void (*pause_end)(void);
    void (*pause_deal)(void);
    void (*block_deal)(void);

    anker_queue_buffer_t save_queue_buf;
    anker_block_buffer_t cur_block_buf;
    anker_block_buffer_t save_block_buf;

    int stop_flag;
    int stop_state;
    int stop_cmd_state;
    int stop_deal_step;
    void (*stop_start)(void);
    void (*stop_end)(void);
    void (*stop_deal)(void);
    void (*clear_stop)(void);

} anker_pause_info_t;

enum anker_stop_status
{
    ANKER_STOP_IDLE,
    ANKER_STOP_START,
};

enum anker_stop_cmd_state
{
    ANKER_STOP_CMD_IDLE,
    ANKER_STOP_CMD_START,
};

void anker_pause_init(void);
anker_pause_info_t *get_anker_pause_info(void);

#endif /* __ANKER_PAUSE_H__ */

#endif /* ANKER_PAUSE_FUNC */

#endif /* ANKER_MAKE_API */
