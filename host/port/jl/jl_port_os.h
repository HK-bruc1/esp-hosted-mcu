/* host/port/jl/jl_port_os.h
 * JL OS API abstraction helpers for ESP-Hosted port layer.
 */

#ifndef JL_PORT_OS_H
#define JL_PORT_OS_H

/* Pull in JL typedefs FIRST — defines u8/u16/u32/u32 and bool. */
#include "generic/typedef.h"

/* Then OS headers that depend on typedefs. */
#include "os/os_api.h"
#include "system/malloc.h"
#include "system/timer.h"

#define JL_OS_TICKS_PER_SEC  OS_TICKS_PER_SEC

static inline int32_t jl_ms_to_ticks(int32_t timeout_ms)
{
    if (timeout_ms < 0) {
        return 0; /* JL: timeout=0 means forever in os_sem_pend/os_mutex_pend */
    }
    if (timeout_ms == 0) {
        return 1; /* poll: smallest non-zero tick */
    }
    return (timeout_ms * JL_OS_TICKS_PER_SEC + 999) / 1000;
}

static inline uint32_t jl_get_ms(void)
{
    return sys_timer_get_ms();
}

#endif /* JL_PORT_OS_H */
