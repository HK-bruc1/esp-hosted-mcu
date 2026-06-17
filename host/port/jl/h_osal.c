/* host/port/jl/h_osal.c
 * JL OSAL contract implementation for ESP-Hosted-MCU Host framework.
 */

#include "h_port_contract.h"
#include "h_port_config.h"
#include "jl_port_os.h"
#include "asm/cpu.h"
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

/* --------------------------------------------------------------------------
 * Memory
 * -------------------------------------------------------------------------- */

static void *jl_malloc(size_t size)
{
    return malloc(size);
}

static void *jl_calloc(size_t n, size_t size)
{
    if (size && n > SIZE_MAX / size) {
        return NULL;
    }
    size_t total = n * size;
    void *p = malloc(total);
    if (p) {
        memset(p, 0, total);
    }
    return p;
}

static void *jl_realloc(void *mem, size_t newsize)
{
    return realloc(mem, newsize);
}

static void jl_free(void *ptr)
{
    free(ptr);
}

static void *jl_memcpy(void *dst, const void *src, size_t n)
{
    return memcpy(dst, src, n);
}

static void *jl_memset(void *s, int c, size_t n)
{
    return memset(s, c, n);
}

static void *jl_malloc_align(size_t size, size_t align)
{
    if (align < sizeof(void *)) {
        align = sizeof(void *);
    }
    size_t alloc_size = size + align + sizeof(void *);
    void *raw = malloc(alloc_size);
    if (!raw) {
        return NULL;
    }
    uintptr_t aligned = ((uintptr_t)raw + sizeof(void *) + align - 1) & ~(align - 1);
    void **back = (void **)aligned - 1;
    *back = raw;
    return (void *)aligned;
}

static void jl_free_align(void *ptr)
{
    if (!ptr) {
        return;
    }
    void **back = (void **)ptr - 1;
    free(*back);
}

/* --------------------------------------------------------------------------
 * Threads
 * -------------------------------------------------------------------------- */

typedef struct {
    char name[16];
    void *handle;
} jl_thread_info_t;

static int jl_thread_create(const char *name, uint32_t prio, uint32_t stack,
                            void (*fn)(void *), void *arg, h_thread_t *out)
{
    if (!out || !fn) {
        return H_ERR_INVALID_ARG;
    }
    jl_thread_info_t *info = (jl_thread_info_t *)malloc(sizeof(jl_thread_info_t));
    if (!info) {
        return H_ERR_NO_MEM;
    }
    memset(info, 0, sizeof(*info));
    if (name) {
        strncpy(info->name, name, sizeof(info->name) - 1);
        info->name[sizeof(info->name) - 1] = '\0';
    }
    /* JL os_task_create expects stack size in u32 words */
    int ret = os_task_create(fn, arg, (u8)prio, stack / sizeof(u32),
                             0, info->name);
    if (ret != 0) {
        free(info);
        return H_FAIL;
    }
    info->handle = os_task_get_handle(info->name);
    *out = (h_thread_t)info;
    return H_OK;
}

static int jl_thread_delete(h_thread_t thread)
{
    if (!thread) {
        return H_ERR_INVALID_ARG;
    }
    jl_thread_info_t *info = (jl_thread_info_t *)thread;
    int ret = os_task_del(info->name);
    free(info);
    return (ret == 0) ? H_OK : H_FAIL;
}

/* --------------------------------------------------------------------------
 * Mutex
 * -------------------------------------------------------------------------- */

static int jl_mutex_create(h_mutex_t *out)
{
    if (!out) {
        return H_ERR_INVALID_ARG;
    }
    OS_MUTEX *m = (OS_MUTEX *)malloc(sizeof(OS_MUTEX));
    if (!m) {
        return H_ERR_NO_MEM;
    }
    if (os_mutex_create(m) != 0) {
        free(m);
        return H_FAIL;
    }
    *out = (h_mutex_t)m;
    return H_OK;
}

static int jl_mutex_lock(h_mutex_t m, int32_t timeout_ms)
{
    if (!m) {
        return H_ERR_INVALID_ARG;
    }
    int ticks = jl_ms_to_ticks(timeout_ms);
    int ret = os_mutex_pend((OS_MUTEX *)m, ticks);
    if (ret == 0) {
        return H_OK;
    }
    if (ticks == 1) {
        return H_ERR_TIMEOUT;
    }
    return H_FAIL;
}

static int jl_mutex_unlock(h_mutex_t m)
{
    if (!m) {
        return H_ERR_INVALID_ARG;
    }
    int ret = os_mutex_post((OS_MUTEX *)m);
    return (ret == 0) ? H_OK : H_FAIL;
}

static int jl_mutex_delete(h_mutex_t m)
{
    if (!m) {
        return H_OK;
    }
    os_mutex_del((OS_MUTEX *)m, OS_DEL_ALWAYS);
    free(m);
    return H_OK;
}

/* --------------------------------------------------------------------------
 * Queue (ring-buffer + mutex + sem)
 * -------------------------------------------------------------------------- */

typedef struct {
    uint8_t  *buf;
    uint32_t  item_size;
    uint32_t  capacity;
    uint32_t  head;
    uint32_t  tail;
    uint32_t  count;
    OS_MUTEX  mutex;
    OS_SEM    sem;
} jl_queue_t;

static int jl_queue_create(uint32_t count, uint32_t item_size, h_queue_t *out)
{
    if (!out || count == 0 || item_size == 0) {
        return H_ERR_INVALID_ARG;
    }
    jl_queue_t *q = (jl_queue_t *)malloc(sizeof(jl_queue_t));
    if (!q) {
        return H_ERR_NO_MEM;
    }
    q->buf = (uint8_t *)malloc(count * item_size);
    if (!q->buf) {
        free(q);
        return H_ERR_NO_MEM;
    }
    q->item_size = item_size;
    q->capacity = count;
    q->head = q->tail = q->count = 0;

    if (os_mutex_create(&q->mutex) != 0 ||
        os_sem_create(&q->sem, 0) != 0) {
        free(q->buf);
        free(q);
        return H_FAIL;
    }
    *out = (h_queue_t)q;
    return H_OK;
}

static int jl_queue_send(h_queue_t q, const void *item, int32_t timeout_ms)
{
    if (!q || !item) {
        return H_ERR_INVALID_ARG;
    }
    jl_queue_t *jq = (jl_queue_t *)q;
    uint32_t start_ms = jl_get_ms();
    int ticks_total = jl_ms_to_ticks(timeout_ms);

    while (1) {
        int ret = os_mutex_pend(&jq->mutex, ticks_total);
        if (ret != 0) {
            return (ticks_total == 1) ? H_ERR_TIMEOUT : H_FAIL;
        }
        if (jq->count < jq->capacity) {
            uint8_t *slot = jq->buf + jq->tail * jq->item_size;
            memcpy(slot, item, jq->item_size);
            jq->tail = (jq->tail + 1) % jq->capacity;
            jq->count++;
            os_mutex_post(&jq->mutex);
            os_sem_post(&jq->sem);
            return H_OK;
        }
        os_mutex_post(&jq->mutex);

        /* Queue is full; wait for a slot to become available. */
        if (timeout_ms == 0) {
            return H_ERR_TIMEOUT;
        }
        if (timeout_ms > 0) {
            uint32_t elapsed = jl_get_ms() - start_ms;
            if (elapsed >= (uint32_t)timeout_ms) {
                return H_ERR_TIMEOUT;
            }
        }
        os_time_dly(1);
    }
}

static int jl_queue_recv(h_queue_t q, void *item, int32_t timeout_ms)
{
    if (!q || !item) {
        return H_ERR_INVALID_ARG;
    }
    jl_queue_t *jq = (jl_queue_t *)q;
    int ticks = jl_ms_to_ticks(timeout_ms);
    int ret = os_sem_pend(&jq->sem, ticks);
    if (ret != 0) {
        return (ticks == 1) ? H_ERR_TIMEOUT : H_FAIL;
    }
    ret = os_mutex_pend(&jq->mutex, 0);
    if (ret != 0) {
        os_sem_post(&jq->sem);
        return H_FAIL;
    }
    uint8_t *slot = jq->buf + jq->head * jq->item_size;
    memcpy(item, slot, jq->item_size);
    jq->head = (jq->head + 1) % jq->capacity;
    jq->count--;
    os_mutex_post(&jq->mutex);
    return H_OK;
}

static int jl_queue_msg_waiting(h_queue_t q)
{
    if (!q) {
        return 0;
    }
    jl_queue_t *jq = (jl_queue_t *)q;
    return (int)jq->count;
}

static int jl_queue_reset(h_queue_t q)
{
    if (!q) {
        return H_ERR_INVALID_ARG;
    }
    jl_queue_t *jq = (jl_queue_t *)q;
    os_mutex_pend(&jq->mutex, 0);
    jq->head = jq->tail = jq->count = 0;
    os_sem_set(&jq->sem, 0);
    os_mutex_post(&jq->mutex);
    return H_OK;
}

static int jl_queue_delete(h_queue_t q)
{
    if (!q) {
        return H_OK;
    }
    jl_queue_t *jq = (jl_queue_t *)q;
    os_mutex_del(&jq->mutex, OS_DEL_ALWAYS);
    os_sem_del(&jq->sem, OS_DEL_ALWAYS);
    free(jq->buf);
    free(jq);
    return H_OK;
}

/* --------------------------------------------------------------------------
 * Semaphore
 * -------------------------------------------------------------------------- */

static int jl_sem_create(uint32_t max, uint32_t init, h_semaphore_t *out)
{
    if (!out) {
        return H_ERR_INVALID_ARG;
    }
    OS_SEM *s = (OS_SEM *)malloc(sizeof(OS_SEM));
    if (!s) {
        return H_ERR_NO_MEM;
    }
    if (os_sem_create(s, (int)init) != 0) {
        free(s);
        return H_FAIL;
    }
    /* max is advisory on uC/OS-II binary/counting semaphores */
    (void)max;
    *out = (h_semaphore_t)s;
    return H_OK;
}

static int jl_sem_take(h_semaphore_t sem, int32_t timeout_ms)
{
    if (!sem) {
        return H_ERR_INVALID_ARG;
    }
    int ticks = jl_ms_to_ticks(timeout_ms);
    int ret = os_sem_pend((OS_SEM *)sem, ticks);
    if (ret == 0) {
        return H_OK;
    }
    return (ticks == 1) ? H_ERR_TIMEOUT : H_FAIL;
}

static int jl_sem_give(h_semaphore_t sem)
{
    if (!sem) {
        return H_ERR_INVALID_ARG;
    }
    int ret = os_sem_post((OS_SEM *)sem);
    return (ret == 0) ? H_OK : H_FAIL;
}

static int jl_sem_give_from_isr(h_semaphore_t sem, void *isr_ctx)
{
    (void)isr_ctx;
    return jl_sem_give(sem);
}

static int jl_sem_delete(h_semaphore_t sem)
{
    if (!sem) {
        return H_OK;
    }
    os_sem_del((OS_SEM *)sem, OS_DEL_ALWAYS);
    free(sem);
    return H_OK;
}

/* --------------------------------------------------------------------------
 * Critical Section
 * -------------------------------------------------------------------------- */

static void jl_enter_critical(void)
{
    OS_ENTER_CRITICAL();
}

static void jl_exit_critical(void)
{
    OS_EXIT_CRITICAL();
}

/* --------------------------------------------------------------------------
 * Timer
 * -------------------------------------------------------------------------- */

typedef enum {
    JL_TIMER_IDLE = 0,      /* created but never started */
    JL_TIMER_ARMED,         /* underlying JL timer active */
    JL_TIMER_IN_CALLBACK,   /* wrapper is executing user callback */
    JL_TIMER_DEFER_FREE,    /* stop requested during callback; free after cb */
    JL_TIMER_STOPPED        /* terminal state, handle can be freed */
} jl_timer_state_t;

typedef struct {
    void (*cb)(void *);
    void *arg;
    u16   id;
    bool  periodic;
    uint32_t period_ms;
    volatile jl_timer_state_t state;
} jl_timer_t;

/* Stop the JL system timer if still armed. Does NOT free the handle. */
static void jl_timer_stop_sys(jl_timer_t *jt)
{
    if (!jt || !jt->id) {
        return;
    }
    if (jt->periodic) {
        sys_timer_del(jt->id);
    } else {
        sys_timeout_del(jt->id);
    }
    jt->id = 0;
}

static void jl_timer_free(jl_timer_t *jt)
{
    if (jt) {
        free(jt);
    }
}

/* Release timer after callback. Must only be called by the wrapper after
 * user callback returns, and only when the timer has not been restarted. */
static void jl_timer_release(jl_timer_t *t)
{
    if (!t) {
        return;
    }

    jl_timer_state_t s = t->state;
    if (s == JL_TIMER_DEFER_FREE || s == JL_TIMER_STOPPED) {
        /* stop()/delete() was called during the callback or timer was already
         * terminal. Free the wrapper now. */
        jl_timer_free(t);
    } else if (s == JL_TIMER_IN_CALLBACK) {
        /* Callback did not change state. For one-shot the timer is consumed.
         * For periodic we keep the wrapper alive because the underlying
         * periodic timer will fire again. */
        if (!t->periodic) {
            t->state = JL_TIMER_STOPPED;
            jl_timer_free(t);
        } else {
            /* Periodic timer remains armed; next expiry is handled by the
             * same wrapper. Mark it armed again so stop() knows it is live. */
            t->state = JL_TIMER_ARMED;
        }
    }
    /* If state is ARMED, the callback restarted the timer; do nothing. */
}

static void jl_timer_wrapper(void *priv)
{
    jl_timer_t *t = (jl_timer_t *)priv;
    if (!t) {
        return;
    }

    /* The underlying timer has fired. Mark that we are inside the callback.
     * If the user callback calls h_timer_stop() or h_timer_delete(), they
     * must not free the wrapper while we are still using it. */
    t->state = JL_TIMER_IN_CALLBACK;

    if (t->cb) {
        t->cb(t->arg);
    }

    /* After callback, decide whether to keep or free the wrapper. The
     * callback may have restarted (state == ARMED) or stopped/deleted
     * (state == DEFER_FREE/STOPPED) the timer. */
    jl_timer_release(t);
}

static int jl_timer_create(const char *name, h_timer_t *out)
{
    (void)name;
    if (!out) {
        return H_ERR_INVALID_ARG;
    }
    jl_timer_t *t = (jl_timer_t *)malloc(sizeof(jl_timer_t));
    if (!t) {
        return H_ERR_NO_MEM;
    }
    memset(t, 0, sizeof(*t));
    *out = (h_timer_t)t;
    return H_OK;
}

static int jl_timer_start(h_timer_t t, uint32_t period_ms, bool periodic,
                          void (*cb)(void *), void *arg)
{
    if (!t || !cb || period_ms == 0) {
        return H_ERR_INVALID_ARG;
    }
    jl_timer_t *jt = (jl_timer_t *)t;

    /* If timer is already armed, stop old instance first. */
    if (jt->state == JL_TIMER_ARMED) {
        jl_timer_stop_sys(jt);
    }

    jt->cb = cb;
    jt->arg = arg;
    jt->periodic = periodic;
    jt->period_ms = period_ms;
    jt->state = JL_TIMER_ARMED;
    if (periodic) {
        jt->id = sys_timer_add(jt, jl_timer_wrapper, period_ms);
    } else {
        jt->id = sys_timeout_add(jt, jl_timer_wrapper, period_ms);
    }
    if (!jt->id) {
        jt->state = JL_TIMER_IDLE;
        return H_FAIL;
    }
    return H_OK;
}

static int jl_timer_stop(h_timer_t t)
{
    if (!t) {
        return H_ERR_INVALID_ARG;
    }
    jl_timer_t *jt = (jl_timer_t *)t;

    switch (jt->state) {
    case JL_TIMER_IDLE:
        /* Never started or start failed; free the wrapper. */
        jl_timer_free(jt);
        return H_OK;
    case JL_TIMER_STOPPED:
        /* Already terminal. */
        return H_OK;
    case JL_TIMER_IN_CALLBACK:
        /* Callback is running. Stop the underlying timer (the one currently
         * executing cannot be cancelled, but this prevents further periodic
         * firings) and ask the wrapper to free after callback returns. */
        jl_timer_stop_sys(jt);
        jt->state = JL_TIMER_DEFER_FREE;
        return H_OK;
    case JL_TIMER_ARMED:
        /* Normal live timer: disarm and free immediately. */
        jl_timer_stop_sys(jt);
        jt->state = JL_TIMER_STOPPED;
        jl_timer_free(jt);
        return H_OK;
    case JL_TIMER_DEFER_FREE:
        /* Already scheduled for free; no-op. */
        return H_OK;
    }
    return H_OK;
}

static int jl_timer_delete(h_timer_t t)
{
    if (!t) {
        return H_OK;
    }
    return jl_timer_stop(t);
}

static uint64_t jl_get_time_ms(void)
{
    return (uint64_t)jl_get_ms();
}

/* --------------------------------------------------------------------------
 * Time / Delay
 * -------------------------------------------------------------------------- */

static void jl_msleep(uint32_t ms)
{
    os_time_dly((int)((ms * JL_OS_TICKS_PER_SEC + 999) / 1000));
}

static void jl_usleep(uint32_t us)
{
    /* JL uC/OS-II lacks microsecond delay; approximate with busy wait.
     * The inner loop count is a rough heuristic for BR28 @ 160 MHz. */
    uint32_t loops = us * 10;
    while (loops--) {
        asm volatile ("nop");
    }
}

static void jl_blocking_delay(unsigned int iterations)
{
    while (iterations--) {
        asm volatile ("nop");
    }
}

/* --------------------------------------------------------------------------
 * Logging
 * -------------------------------------------------------------------------- */

static void jl_log_write(int level, const char *tag, const char *fmt, ...)
{
    (void)level;
    va_list ap;
    va_start(ap, fmt);
    printf("[%s] ", tag ? tag : "???");
    vprintf(fmt, ap);
    printf("\n");
    va_end(ap);
}

/* --------------------------------------------------------------------------
 * Contract export
 * -------------------------------------------------------------------------- */

const h_osal_contract_t g_h_osal = {
    .malloc       = jl_malloc,
    .calloc       = jl_calloc,
    .realloc      = jl_realloc,
    .free         = jl_free,
    .memcpy       = jl_memcpy,
    .memset       = jl_memset,
    .malloc_align = jl_malloc_align,
    .free_align   = jl_free_align,

    .thread_create = jl_thread_create,
    .thread_delete = jl_thread_delete,

    .mutex_create = jl_mutex_create,
    .mutex_lock   = jl_mutex_lock,
    .mutex_unlock = jl_mutex_unlock,
    .mutex_delete = jl_mutex_delete,

    .queue_create      = jl_queue_create,
    .queue_send        = jl_queue_send,
    .queue_recv        = jl_queue_recv,
    .queue_msg_waiting = jl_queue_msg_waiting,
    .queue_reset       = jl_queue_reset,
    .queue_delete      = jl_queue_delete,

    .sem_create       = jl_sem_create,
    .sem_take         = jl_sem_take,
    .sem_give         = jl_sem_give,
    .sem_give_from_isr = jl_sem_give_from_isr,
    .sem_delete       = jl_sem_delete,

    .enter_critical = jl_enter_critical,
    .exit_critical  = jl_exit_critical,

    .timer_create  = jl_timer_create,
    .timer_start   = jl_timer_start,
    .timer_stop    = jl_timer_stop,
    .timer_delete  = jl_timer_delete,
    .get_time_ms   = jl_get_time_ms,

    .msleep         = jl_msleep,
    .usleep         = jl_usleep,
    .blocking_delay = jl_blocking_delay,

    .log_write = jl_log_write,
};
