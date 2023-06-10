#include "soft_timer.h"

static soft_timer_t *soft_timer_head = 0;

int soft_timer_init(soft_timer_t *timer, uint32_t timeout_ms, uint32_t repeat, void (*callback)(void))
{
    if (timer == 0 || timeout_ms == 0)
    {
        return -1;
    }

    timer->timeout_ms = timeout_ms;
    timer->start_ms = 0;
    timer->repeat = repeat;
    timer->repeat_cnt = 0;
    timer->callback = callback;
    timer->next = 0;

    return 0;
}

int soft_timer_start(soft_timer_t *timer)
{
    soft_timer_t *target;

    for (target = soft_timer_head; target; target = target->next)
    {
        if (target == timer)
        {
            return -1;
        }
    }

    timer->repeat_cnt = 0;
    timer->start_ms = soft_timer_get_ms();

    timer->next = soft_timer_head;
    soft_timer_head = timer;

    return 0;
}

int soft_timer_stop(soft_timer_t *timer)
{
    soft_timer_t **curr;

    for (curr = &soft_timer_head; *curr; curr = &(*curr)->next)
    {
        if (*curr == timer)
        {
            *curr = timer->next;
            return 0;
        }
    }
    return -1;
}

void soft_timer_loop(void)
{
    soft_timer_t *target;
    uint32_t timebase_ms;

    for (target = soft_timer_head; target; target = target->next)
    {
        timebase_ms = soft_timer_get_ms();

        if (timebase_ms - target->start_ms < target->timeout_ms)
        {
            continue;
        }

        target->callback();

        if (target->repeat == 0)
        {
            target->start_ms = timebase_ms;
            continue;
        }

        target->repeat_cnt += 1;
        if (target->repeat_cnt < target->repeat)
        {
            target->start_ms = timebase_ms;
            continue;
        }
        soft_timer_stop(target);
    }
}
