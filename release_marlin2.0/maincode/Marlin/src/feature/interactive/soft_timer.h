#ifndef __SOFT_TIMER_H__
#define __SOFT_TIMER_H__

#include <stdint.h>
#include "clock.h"

#define soft_timer_get_ms() getCurrentMillis()

#define SOFT_TIMER_REPEAT_FOREVER 0

typedef struct soft_timer
{
    uint32_t timeout_ms;
    uint32_t start_ms;
    uint32_t repeat;
    uint32_t repeat_cnt;
    void (*callback)(void);
    struct soft_timer *next;
} soft_timer_t;

int soft_timer_init(soft_timer_t *timer, uint32_t timeout_ms, uint32_t repeat, void (*callback)(void));

int soft_timer_start(soft_timer_t *timer);

int soft_timer_stop(soft_timer_t *timer);

void soft_timer_loop(void);

#endif
