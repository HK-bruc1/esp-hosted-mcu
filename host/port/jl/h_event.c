/* host/port/jl/h_event.c
 * JL event contract implementation.
 */

#include "h_port_contract.h"
#include "h_wrapper.h"
#include "h_port_config.h"
#include "jl_port_os.h"
#include <string.h>

typedef struct event_handler_entry {
    struct event_handler_entry *next;
    h_event_base_t              base;
    int32_t                     event_id;
    h_event_handler_t           handler;
    void                       *user_ctx;
} event_handler_entry_t;

static event_handler_entry_t *g_handlers = NULL;
static h_mutex_t              g_event_lock = NULL;

static int jl_event_register(h_event_base_t base, int32_t event_id,
                             h_event_handler_t handler, void *user_ctx)
{
    if (!handler || base >= H_EVENT_MAX) {
        return H_ERR_INVALID_ARG;
    }
    if (!g_event_lock) {
        return H_ERR_INVALID_STATE;
    }

    event_handler_entry_t *e = (event_handler_entry_t *)
        h_malloc(sizeof(event_handler_entry_t));
    if (!e) {
        return H_ERR_NO_MEM;
    }
    e->next = NULL;
    e->base = base;
    e->event_id = event_id;
    e->handler = handler;
    e->user_ctx = user_ctx;

    h_mutex_lock(g_event_lock, H_BLOCK_MAX);
    e->next = g_handlers;
    g_handlers = e;
    h_mutex_unlock(g_event_lock);
    return H_OK;
}

static int jl_event_unregister(h_event_base_t base, int32_t event_id,
                               h_event_handler_t handler)
{
    if (!handler || base >= H_EVENT_MAX || !g_event_lock) {
        return H_ERR_INVALID_ARG;
    }

    h_mutex_lock(g_event_lock, H_BLOCK_MAX);
    event_handler_entry_t **pp = &g_handlers;
    while (*pp) {
        if ((*pp)->base == base &&
            (*pp)->event_id == event_id &&
            (*pp)->handler == handler) {
            event_handler_entry_t *tmp = *pp;
            *pp = (*pp)->next;
            h_mutex_unlock(g_event_lock);
            h_free(tmp);
            return H_OK;
        }
        pp = &(*pp)->next;
    }
    h_mutex_unlock(g_event_lock);
    return H_ERR_INVALID_ARG;
}

static int jl_event_post(h_event_base_t base, int32_t event_id,
                         void *event_data, size_t event_data_size)
{
    if (base >= H_EVENT_MAX || !g_event_lock) {
        return H_ERR_INVALID_ARG;
    }

    /* Snapshot matching handlers without invoking them under the lock,
     * so a callback can safely register/unregister handlers. */
#define JL_EVENT_MAX_HANDLERS_PER_POST  8
    struct {
        h_event_handler_t handler;
        void *user_ctx;
    } snapshot[JL_EVENT_MAX_HANDLERS_PER_POST];
    int n = 0;

    h_mutex_lock(g_event_lock, H_BLOCK_MAX);
    event_handler_entry_t *p = g_handlers;
    while (p && n < JL_EVENT_MAX_HANDLERS_PER_POST) {
        if (p->base == base && p->event_id == event_id) {
            snapshot[n].handler = p->handler;
            snapshot[n].user_ctx = p->user_ctx;
            n++;
        }
        p = p->next;
    }
    h_mutex_unlock(g_event_lock);

    for (int i = 0; i < n; i++) {
        snapshot[i].handler(event_data, event_data_size, snapshot[i].user_ctx);
    }
    return H_OK;
#undef JL_EVENT_MAX_HANDLERS_PER_POST
}

static int jl_event_wifi_post(int32_t event_id, void *event_data,
                              size_t event_data_size, int32_t timeout_ms)
{
    (void)timeout_ms;
    return jl_event_post(H_EVENT_WIFI, event_id, event_data, event_data_size);
}

const h_event_contract_t g_h_event = {
    .register_handler = jl_event_register,
    .unregister_handler = jl_event_unregister,
    .post = jl_event_post,
    .wifi_post = jl_event_wifi_post,
};

h_err_t h_port_event_init(void)
{
    if (g_event_lock) {
        return H_OK;
    }
    return h_mutex_create(&g_event_lock);
}

void h_port_event_deinit(void)
{
    if (!g_event_lock) {
        return;
    }
    h_mutex_lock(g_event_lock, H_BLOCK_MAX);
    while (g_handlers) {
        event_handler_entry_t *tmp = g_handlers;
        g_handlers = g_handlers->next;
        h_free(tmp);
    }
    h_mutex_unlock(g_event_lock);
    h_mutex_delete(g_event_lock);
    g_event_lock = NULL;
}
