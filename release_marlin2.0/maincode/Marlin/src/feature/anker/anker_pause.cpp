/*
 * @Author       : winter
 * @Date         : 2022-04-02 10:50:23
 * @LastEditors  : winter
 * @LastEditTime : 2022-05-20 14:03:20
 * @Description  :
 */
#include "anker_pause.h"

#if ENABLED(ANKER_MAKE_API)

#if ENABLED(ANKER_PAUSE_FUNC)

#include "../../gcode/queue.h"
#include "../../gcode/gcode.h"
#include "../../module/planner.h"
#include "../../module/stepper.h"
#include "../../module/temperature.h"

#define ANKER_PAUSE_RESET 0       //anker continue after pause reset enable/disable
#define ANKER_PAUSE_PRE_EXTRUDE 0 //anker continue after pause pre-extrude enable/disable

enum anker_pause_deal_step
{
    ANKER_PAUSE_DEAL_STEP_IDLE = 0,
    ANKER_PAUSE_DEAL_STEP_SAVE_QUEUE,
    ANKER_PAUSE_DEAL_STEP_SAVE_BLOCK,
    ANKER_PAUSE_DEAL_STEP_CLEAR_DATA,
    ANKER_PAUSE_DEAL_STEP_PAUSE_OK,
    ANKER_PAUSE_DEAL_STEP_RECOVER_PRE,
    ANKER_PAUSE_DEAL_STEP_RECOVER_BLOCK,
    ANKER_PAUSE_DEAL_STEP_RECOVER_QUEUE,
    ANKER_PAUSE_DEAL_STEP_END,
};

enum anker_stop_deal_step
{
    ANKER_STOP_DEAL_STEP_IDLE = 0,
    ANKER_STOP_DEAL_STEP_CLEAR,
    ANKER_STOP_DEAL_STEP_STOP_OK,
    ANKER_STOP_DEAL_STEP_END,
};

static void anker_pause_start(void)
{
    anker_pause_info_t *p_info = get_anker_pause_info();

    p_info->pause_cmd_state = ANKER_PAUSE_CMD_START;

    if (p_info->pause_flag != ANKER_PAUSE_START)
    {
        p_info->pause_state = ANKER_PAUSE_START;
        p_info->pause_serial_state = ANKER_PAUSE_SERIAL_ENABLE;
        p_info->pause_queue_state = ANKER_PAUSE_QUEUE_ENABLE;
        // p_info->pause_block_state = ANKER_PAUSE_BLOCK_ENABLE;
        p_info->pause_deal_step = ANKER_PAUSE_DEAL_STEP_IDLE;

        p_info->pause_flag = ANKER_PAUSE_START;
    }
}

static void anker_pause_continue(void)
{
    anker_pause_info_t *p_info = get_anker_pause_info();
    p_info->pause_cmd_state = ANKER_PAUSE_CMD_CONTINUE;
}

static void anker_pause_end(void)
{
    anker_pause_info_t *p_info = get_anker_pause_info();

    if (p_info->pause_flag != ANKER_PAUSE_IDLE)
    {
        p_info->pause_state = ANKER_PAUSE_IDLE;
        p_info->pause_serial_state = ANKER_PAUSE_IDLE;
        p_info->pause_queue_state = ANKER_PAUSE_IDLE;
        p_info->pause_block_state = ANKER_PAUSE_IDLE;
        p_info->pause_cmd_state = ANKER_PAUSE_CMD_IDLE;
        p_info->pause_deal_step = ANKER_PAUSE_DEAL_STEP_IDLE;

        p_info->pause_flag = ANKER_PAUSE_IDLE;
    }
}

static void anker_pause_save_buf_init(void)
{
    unsigned int i = 0;

    anker_pause_info_t *p_info = get_anker_pause_info();

    //queue save buf init
    p_info->save_queue_buf.length = 0;
    p_info->save_queue_buf.index_r = 0;
    p_info->save_queue_buf.index_w = 0;
    for (i = 0; i < BUFSIZE; i++)
    {
        memset(p_info->save_queue_buf.commands[i].buffer, 0, MAX_CMD_SIZE);
    }
}

static void anker_pause_save_queue_buf_advance_pos(uint16_t &p, const int inc)
{
    anker_pause_info_t *p_info = get_anker_pause_info();

    if (++p >= BUFSIZE)
    {
        p = 0;
    }
    p_info->save_queue_buf.length += inc;
}

static uint8_t anker_pause_next_block_index(const uint8_t block_index)
{
    return BLOCK_MOD(block_index + 1);
}

static void anker_pause_block_deal(void)
{
    anker_pause_info_t *p_info = get_anker_pause_info();

    memcpy(&p_info->cur_block_buf.cur_pos, &current_position, sizeof(xyze_pos_t));
    p_info->cur_block_buf.cur_fr_mm_s = feedrate_mm_s;
    p_info->cur_block_buf.block_length = planner.movesplanned();
    p_info->cur_block_buf.block_info_head = planner.block_buffer_head;
    p_info->cur_block_buf.block_info_tail = planner.block_buffer_tail;
    // memcpy(&(p_info->save_block_buf), &(p_info->cur_block_buf), sizeof(anker_block_buffer_t));

    planner.clear_block_buffer(); // block清除
}

static void anker_pause_deal(void)
{
    anker_pause_info_t *p_info = get_anker_pause_info();

    if ((p_info->pause_cmd_state == ANKER_PAUSE_CMD_START) && (p_info->pause_state == ANKER_PAUSE_CONTINUE))
    {
        queue.clear();
        planner.clear_block_buffer();
        planner.quick_stop();
        p_info->pause_deal_step = ANKER_PAUSE_DEAL_STEP_CLEAR_DATA;
    }

    switch (p_info->pause_deal_step)
    {
    case ANKER_PAUSE_DEAL_STEP_IDLE:
    {
        if (p_info->pause_state == ANKER_PAUSE_START)
        {
            p_info->pause_deal_step = ANKER_PAUSE_DEAL_STEP_SAVE_QUEUE;
        }
        break;
    }
    case ANKER_PAUSE_DEAL_STEP_SAVE_QUEUE:
    {
        if (p_info->pause_queue_state == ANKER_PAUSE_QUEUE_OK)
        {
            anker_pause_save_buf_init();

            while (!(queue.ring_buffer.empty()))
            {
                strcpy(p_info->save_queue_buf.commands[p_info->save_queue_buf.index_w].buffer, queue.ring_buffer.peek_next_command_string());
                p_info->save_queue_buf.commands[p_info->save_queue_buf.index_w].skip_ok = queue.ring_buffer.commands[queue.ring_buffer.index_r].skip_ok;
                p_info->save_queue_buf.commands[p_info->save_queue_buf.index_w].port.index = queue.ring_buffer.commands[queue.ring_buffer.index_r].port.index;
                anker_pause_save_queue_buf_advance_pos(p_info->save_queue_buf.index_w, 1);
                queue.ring_buffer.advance_pos(queue.ring_buffer.index_r, -1);
            }

            memset(p_info->tmp_cmd_buf, 0, sizeof(p_info->tmp_cmd_buf));
            sprintf(p_info->tmp_cmd_buf, "<== %d ==> : queue_length = %d --- %d\r\n", __LINE__, p_info->save_queue_buf.length, queue.ring_buffer.length);
            MYSERIAL1.printf(p_info->tmp_cmd_buf);

            p_info->pause_deal_step = ANKER_PAUSE_DEAL_STEP_SAVE_BLOCK;
        }
        break;
    }
    case ANKER_PAUSE_DEAL_STEP_SAVE_BLOCK:
    {
        if (p_info->pause_block_state == ANKER_PAUSE_BLOCK_OK)
        {
            MYSERIAL1.printf("ANKER_PAUSE_BLOCK_OK\r\n");

            // memcpy(&p_info->cur_block_buf.cur_pos, &current_position, sizeof(xyze_pos_t));
            // p_info->cur_block_buf.cur_fr_mm_s = feedrate_mm_s;
            // p_info->cur_block_buf.block_length = planner.movesplanned();
            // p_info->cur_block_buf.block_info_head = planner.block_buffer_head;
            // p_info->cur_block_buf.block_info_tail = planner.block_buffer_tail;
            memcpy(&(p_info->save_block_buf), &(p_info->cur_block_buf), sizeof(anker_block_buffer_t));

            memset(p_info->tmp_cmd_buf, 0, sizeof(p_info->tmp_cmd_buf));
            sprintf(p_info->tmp_cmd_buf, "<== %d ==> : block_length = %d\r\n", __LINE__, p_info->save_block_buf.block_length);
            MYSERIAL1.printf(p_info->tmp_cmd_buf);
            memset(p_info->tmp_cmd_buf, 0, sizeof(p_info->tmp_cmd_buf));
            sprintf(p_info->tmp_cmd_buf, "<== %d ==> : x = %f y = %f z = %f e = %f fr = %f\r\n", __LINE__,
                    p_info->save_block_buf.cur_pos.x,
                    p_info->save_block_buf.cur_pos.y,
                    p_info->save_block_buf.cur_pos.z,
                    p_info->save_block_buf.cur_pos.e,
                    p_info->save_block_buf.cur_fr_mm_s);
            MYSERIAL1.printf(p_info->tmp_cmd_buf);

            p_info->pause_deal_step = ANKER_PAUSE_DEAL_STEP_CLEAR_DATA;
        }

        if (!planner.has_blocks_queued())
        {
            // memcpy(&p_info->save_block_buf.cur_pos, &current_position, sizeof(xyze_pos_t));
            p_info->save_block_buf.cur_pos.x = current_position.asLogical().x;
            p_info->save_block_buf.cur_pos.y = current_position.asLogical().y;
            p_info->save_block_buf.cur_pos.z = current_position.asLogical().z;
            p_info->save_block_buf.cur_pos.e = current_position.asLogical().e;
            p_info->save_block_buf.cur_fr_mm_s = feedrate_mm_s * 60;

            // MYSERIAL1.printf(" ==OC=== x=%f, y=%f, z=%f, e=%f, ===C==\r\n", current_position.x, 
            // current_position.y, current_position.z, current_position.e);
            // MYSERIAL1.printf(" ===== x=%f, y=%f, z=%f, e=%f, f=%f =====\r\n", p_info->save_block_buf.cur_pos.x, 
            // p_info->save_block_buf.cur_pos.y, p_info->save_block_buf.cur_pos.z, p_info->save_block_buf.cur_pos.e,
            // p_info->save_block_buf.cur_fr_mm_s);
            // MYSERIAL1.printf(" ==C=== x=%f, y=%f, z=%f, e=%f, ===C==\r\n", current_position.x, 
            // current_position.y, current_position.z, current_position.e);
            // MYSERIAL1.printf(" ==A=== x=%f, y=%f, z=%f, e=%f, ===C==\r\n", current_position.asLogical().x, 
            // current_position.asLogical().y, current_position.asLogical().z, current_position.e);

            p_info->pause_deal_step = ANKER_PAUSE_DEAL_STEP_CLEAR_DATA;
        }
        break;
    }
    case ANKER_PAUSE_DEAL_STEP_CLEAR_DATA:
    {
        queue.clear();
        // planner.clear_block_buffer();
        p_info->pause_block_state = ANKER_PAUSE_BLOCK_DISABLE;
        p_info->pause_queue_state = ANKER_PAUSE_QUEUE_DISABLE;
        queue.ring_buffer.enqueue("G1 X10 Y0 F15000\r\n");
        // MYSERIAL1.printf("go zero\r\n");
        p_info->pause_serial_state = ANKER_PAUSE_SERIAL_DISABLE;

        p_info->pause_state = ANKER_PAUSE_OK;
        p_info->pause_deal_step = ANKER_PAUSE_DEAL_STEP_PAUSE_OK;
        break;
    }
    case ANKER_PAUSE_DEAL_STEP_PAUSE_OK:
    {
        if (p_info->pause_cmd_state == ANKER_PAUSE_CMD_CONTINUE)
        {
            p_info->pause_state = ANKER_PAUSE_CONTINUE;
            p_info->pause_serial_state = ANKER_PAUSE_SERIAL_ENABLE;

            memset(p_info->tmp_cmd_buf, 0, sizeof(p_info->tmp_cmd_buf));
            #if ENABLED(ANKER_PAUSE_RESET)
            sprintf(p_info->tmp_cmd_buf, "G28 X0 Y0 F15000\r\n");
            queue.ring_buffer.enqueue(p_info->tmp_cmd_buf);
            #endif
            #if ENABLED(ANKER_PAUSE_PRE_EXTRUDE)
            sprintf(p_info->tmp_cmd_buf, "G92 E%f\r\n", p_info->save_block_buf.cur_pos.e - 20);
            queue.ring_buffer.enqueue(p_info->tmp_cmd_buf);
            sprintf(p_info->tmp_cmd_buf, "G1 E%f F500\r\n", p_info->save_block_buf.cur_pos.e);
            queue.ring_buffer.enqueue(p_info->tmp_cmd_buf);
            #endif
            sprintf(p_info->tmp_cmd_buf, "G1 X%f Y%f Z%f F15000\r\n", p_info->save_block_buf.cur_pos.x,
                    p_info->save_block_buf.cur_pos.y, p_info->save_block_buf.cur_pos.z);
            queue.ring_buffer.enqueue(p_info->tmp_cmd_buf);
            // MYSERIAL1.printf("<==> CONTINUE %s",p_info->tmp_cmd_buf);
            sprintf(p_info->tmp_cmd_buf, "G1 F%f\r\n", p_info->save_block_buf.cur_fr_mm_s);
            queue.ring_buffer.enqueue(p_info->tmp_cmd_buf);

            p_info->pause_deal_step = ANKER_PAUSE_DEAL_STEP_RECOVER_PRE;
        }
        break;
    }
    case ANKER_PAUSE_DEAL_STEP_RECOVER_PRE:
    {
        if (queue.ring_buffer.empty() && !planner.has_blocks_queued())
        {
            p_info->tmp_block_tail = p_info->save_block_buf.block_info_tail;
            p_info->tmp_block_length = p_info->save_block_buf.block_length;

            // p_info->pause_deal_step = ANKER_PAUSE_DEAL_STEP_RECOVER_BLOCK;
            p_info->pause_deal_step = ANKER_PAUSE_DEAL_STEP_RECOVER_QUEUE;
        }
        break;
    }
    case ANKER_PAUSE_DEAL_STEP_RECOVER_BLOCK:
    {
        if (p_info->tmp_block_length == 0)
        {
            p_info->pause_deal_step = ANKER_PAUSE_DEAL_STEP_RECOVER_QUEUE;
        }
        else
        {
            while (!(planner.is_full()) && p_info->tmp_block_length != 0)
            {
                planner.buffer_line(p_info->save_block_buf.block_info[p_info->tmp_block_tail].xyze_pos,
                                    p_info->save_block_buf.block_info[p_info->tmp_block_tail].feed_rate_mm_s,
                                    p_info->save_block_buf.block_info[p_info->tmp_block_tail].extruder,
                                    p_info->save_block_buf.block_info[p_info->tmp_block_tail].millimeters);

                p_info->tmp_block_tail = anker_pause_next_block_index(p_info->tmp_block_tail);
                p_info->tmp_block_length--;
            }
        }
        break;
    }
    case ANKER_PAUSE_DEAL_STEP_RECOVER_QUEUE:
    {
        if (p_info->save_queue_buf.length == 0)
        {
            p_info->pause_serial_state = ANKER_PAUSE_SERIAL_DISABLE;
            p_info->pause_deal_step = ANKER_PAUSE_DEAL_STEP_END;
        }
        else
        {
            while (!(queue.ring_buffer.full()) && (p_info->save_queue_buf.length != 0))
            {
                queue.ring_buffer.enqueue(p_info->save_queue_buf.commands[p_info->save_queue_buf.index_r].buffer,
                                          p_info->save_queue_buf.commands[p_info->save_queue_buf.index_r].skip_ok,
                                          p_info->save_queue_buf.commands[p_info->save_queue_buf.index_r].port.index);

                anker_pause_save_queue_buf_advance_pos(p_info->save_queue_buf.index_r, -1);
            }
        }
        break;
    }
    case ANKER_PAUSE_DEAL_STEP_END:
    {
        if (p_info->pause_cmd_state == ANKER_PAUSE_CMD_START)
        {
            p_info->pause_deal_step = ANKER_PAUSE_DEAL_STEP_SAVE_QUEUE;
        }
        else
        {
            p_info->pause_end();
            p_info->pause_deal_step = ANKER_PAUSE_DEAL_STEP_IDLE;
        }
        break;
    }

    default:
    {
        p_info->pause_deal_step = ANKER_PAUSE_DEAL_STEP_IDLE;
        break;
    }
    }
}

static void anker_stop_start(void)
{
    anker_pause_info_t *p_info = get_anker_pause_info();

    p_info->stop_cmd_state = ANKER_STOP_CMD_START;

    if (p_info->stop_flag != ANKER_STOP_START)
    {
        p_info->stop_state = ANKER_STOP_START;
        p_info->pause_serial_state = ANKER_PAUSE_SERIAL_ENABLE;
        p_info->stop_deal_step = ANKER_STOP_DEAL_STEP_IDLE;

        p_info->stop_flag = ANKER_STOP_START;
    }
}

static void anker_stop_end(void)
{
    anker_pause_info_t *p_info = get_anker_pause_info();

    if (p_info->stop_flag != ANKER_STOP_IDLE)
    {
        p_info->stop_state = ANKER_STOP_IDLE;
        p_info->pause_serial_state = ANKER_PAUSE_IDLE;
        p_info->stop_cmd_state = ANKER_STOP_CMD_IDLE;
        p_info->stop_deal_step = ANKER_STOP_DEAL_STEP_IDLE;

        p_info->stop_flag = ANKER_STOP_IDLE;
    }
}

static void anker_stop_deal(void)
{
    anker_pause_info_t *p_info = get_anker_pause_info();

    switch(p_info->stop_deal_step)
    {
        case ANKER_STOP_DEAL_STEP_IDLE:
        {
            if (p_info->stop_state == ANKER_STOP_START)
            {
                p_info->stop_deal_step = ANKER_STOP_DEAL_STEP_CLEAR;
            }
            break;
        }
        case ANKER_STOP_DEAL_STEP_CLEAR:
        {
            queue.clear();
            // planner.clear_block_buffer();
            quickstop_stepper();
            thermalManager.disable_all_heaters();
            thermalManager.zero_fan_speeds();
            p_info->stop_deal_step = ANKER_STOP_DEAL_STEP_STOP_OK;
            break;
        }
        case ANKER_STOP_DEAL_STEP_STOP_OK:
        {
            p_info->pause_serial_state = ANKER_PAUSE_SERIAL_DISABLE;
            p_info->stop_deal_step = ANKER_STOP_DEAL_STEP_END;
            break;
        }
        case ANKER_STOP_DEAL_STEP_END:
        {
            p_info->pause_end();
            p_info->stop_end();
            p_info->stop_deal_step = ANKER_STOP_DEAL_STEP_IDLE;
            break;
        }

        default:
        {
            p_info->stop_deal_step = ANKER_STOP_DEAL_STEP_IDLE;
            break;
        }
    }
}

void anker_pause_init(void)
{
    anker_pause_info_t *p_info = get_anker_pause_info();

    p_info->pause_start = anker_pause_start;
    p_info->pause_continue = anker_pause_continue;
    p_info->pause_end = anker_pause_end;
    p_info->pause_deal = anker_pause_deal;
    p_info->block_deal = anker_pause_block_deal;

    p_info->stop_start = anker_stop_start;
    p_info->stop_end = anker_stop_end;
    p_info->stop_deal = anker_stop_deal;
}

anker_pause_info_t *get_anker_pause_info(void)
{
    static anker_pause_info_t anker_pause_info = {0};
    return &anker_pause_info;
}

#endif /* ANKER_PAUSE_FUNC */

#endif /* ANKER_MAKE_API */
