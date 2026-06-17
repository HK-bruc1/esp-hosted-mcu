/* host/port/jl/mempool.h
 * JL port shadow of common/mempool/include/mempool.h
 * Avoids pulling in sys/queue.h, FreeRTOS headers, and sdkconfig.h.
 */

#ifndef __MEMPOOL_H__
#define __MEMPOOL_H__

#include <stdint.h>
#include <stddef.h>

typedef struct hosted_mempool_t hosted_mempool_t;

typedef enum {
    HOSTED_MEM_CAP_NONE,
    HOSTED_MEM_CAP_DMA,
    HOSTED_MEM_CAP_MAX
} hosted_mem_cap_t;

#endif /* __MEMPOOL_H__ */
