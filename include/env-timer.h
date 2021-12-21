#ifndef HOST_TIMER_H_H8PV4PLO
#define HOST_TIMER_H_H8PV4PLO

#include <signal.h>
#include <stdbool.h>
#include <uapi/timer-def.h>

typedef void (*TimerHandler)();
timer_id setup_timer(TimerHandler handler);
void soonest_interrupt_ns(long ns);
void block_interrupt();
void unblock_interrupt();
bool is_interrupt_blocked();
void fire_timer();

void leave_interrpt_context();
void enter_interrpt_context();

#endif /* end of include guard: HOST_TIMER_H_H8PV4PLO */
