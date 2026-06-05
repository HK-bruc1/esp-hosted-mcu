/* host/port/esp-idf/h_osal.c
 * ESP-IDF OSAL port — maps h_osal_contract_t to ESP-IDF/FreeRTOS APIs.
 *
 * Each function pointer in g_h_osal is filled with an adapter that translates
 * between the portable h_types.h signatures and the platform-native
 * ESP-IDF / FreeRTOS APIs. The contract struct is a single const global
 * — no dynamic dispatch, no vtables, negligible overhead. */

#include "h_port_contract.h"
#include "h_port_config.h"

/* ESP-IDF / FreeRTOS headers */
#include <stdlib.h>
#include <string.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>
#include <freertos/queue.h>
#include <freertos/timers.h>
#include <esp_timer.h>
#include <esp_heap_caps.h>
#include <esp_log.h>
#include <esp_system.h>   /* esp_restart(), esp_unregister_shutdown_handler */
#include <esp_wifi.h>     /* esp_wifi_stop */
#include <esp_sleep.h>    /* esp_sleep_get_wakeup_cause, esp_deep_sleep_* */
#include "esp_hosted_power_save.h"
#include <stdarg.h>

/* Weak fallback for SPI-HD extension — overridden by real implementation
 * in port_esp_hosted_host_spi_hd.c when CONFIG_ESP_HOSTED_SPI_HD_HOST_INTERFACE
 * is enabled.  In all other transport configs the linker resolves to this stub. */
__attribute__((weak)) int hosted_spi_hd_set_data_lines(uint32_t data_lines)
{
    (void)data_lines;
    return H_ERR_NOT_SUP;
}

/* ──  Helpers ── */

/* Convert timeout_ms to FreeRTOS ticks.
 * Negative means block indefinitely (portMAX_DELAY). */
static TickType_t timeout_to_ticks(int32_t timeout_ms)
{
    if (timeout_ms < 0) return portMAX_DELAY;
    return pdMS_TO_TICKS((uint32_t)timeout_ms);
}

/* ──  Memory ── */

static void *h_malloc_align_adapter(size_t size, size_t align)
{
    /* heap_caps_aligned_alloc takes (alignment, size, caps) — reorder params */
    return heap_caps_aligned_alloc(align, size,
            MALLOC_CAP_INTERNAL | MALLOC_CAP_DMA | MALLOC_CAP_8BIT);
}

/* ──  Threads ── */

static int h_thread_create_adapter(const char *name, uint32_t prio,
                                   uint32_t stack, void (*fn)(void*),
                                   void *arg, h_thread_t *out)
{
    TaskHandle_t handle = NULL;
    BaseType_t ret = xTaskCreate(fn, name, stack, arg, prio, &handle);
    *out = (h_thread_t)handle;
    return (ret == pdPASS) ? H_OK : H_FAIL;
}

static int h_thread_delete_adapter(h_thread_t thread)
{
    vTaskDelete((TaskHandle_t)thread);
    return H_OK;
}

/* ──  Mutex ── */

static int h_mutex_create_adapter(h_mutex_t *out)
{
    SemaphoreHandle_t sem = xSemaphoreCreateMutex();
    *out = (h_mutex_t)sem;
    return (sem != NULL) ? H_OK : H_ERR_NO_MEM;
}

static int h_mutex_lock_adapter(h_mutex_t m, int32_t timeout_ms)
{
    TickType_t ticks = timeout_to_ticks(timeout_ms);
    BaseType_t ret = xSemaphoreTake((SemaphoreHandle_t)m, ticks);
    return (ret == pdTRUE) ? H_OK : H_ERR_TIMEOUT;
}

static int h_mutex_unlock_adapter(h_mutex_t m)
{
    BaseType_t ret = xSemaphoreGive((SemaphoreHandle_t)m);
    return (ret == pdTRUE) ? H_OK : H_FAIL;
}

static int h_mutex_delete_adapter(h_mutex_t m)
{
    xSemaphoreGive((SemaphoreHandle_t)m);
    vSemaphoreDelete((SemaphoreHandle_t)m);
    return H_OK;
}

/* ──  Queue ── */

static int h_queue_create_adapter(uint32_t count, uint32_t item_size,
                                  h_queue_t *out)
{
    QueueHandle_t q = xQueueCreate(count, item_size);
    *out = (h_queue_t)q;
    return (q != NULL) ? H_OK : H_ERR_NO_MEM;
}

static int h_queue_send_adapter(h_queue_t q, const void *item,
                                int32_t timeout_ms)
{
    TickType_t ticks = timeout_to_ticks(timeout_ms);
    BaseType_t ret = xQueueSendToBack((QueueHandle_t)q, item, ticks);
    return (ret == pdTRUE) ? H_OK : H_ERR_TIMEOUT;
}

static int h_queue_recv_adapter(h_queue_t q, void *item, int32_t timeout_ms)
{
    TickType_t ticks = timeout_to_ticks(timeout_ms);
    BaseType_t ret = xQueueReceive((QueueHandle_t)q, item, ticks);
    return (ret == pdTRUE) ? H_OK : H_ERR_TIMEOUT;
}

static int h_queue_msg_waiting_adapter(h_queue_t q)
{
    return (int)uxQueueMessagesWaiting((QueueHandle_t)q);
}

static int h_queue_reset_adapter(h_queue_t q)
{
    xQueueReset((QueueHandle_t)q);
    return H_OK;
}

static int h_queue_delete_adapter(h_queue_t q)
{
    vQueueDelete((QueueHandle_t)q);
    return H_OK;
}

/* ──  Semaphore ── */

static int h_sem_create_adapter(uint32_t max, uint32_t init,
                                h_semaphore_t *out)
{
    SemaphoreHandle_t sem = xSemaphoreCreateCounting(max, init);
    *out = (h_semaphore_t)sem;
    return (sem != NULL) ? H_OK : H_ERR_NO_MEM;
}

static int h_sem_take_adapter(h_semaphore_t sem, int32_t timeout_ms)
{
    TickType_t ticks = timeout_to_ticks(timeout_ms);
    BaseType_t ret = xSemaphoreTake((SemaphoreHandle_t)sem, ticks);
    return (ret == pdTRUE) ? H_OK : H_ERR_TIMEOUT;
}

static int h_sem_give_adapter(h_semaphore_t sem)
{
    BaseType_t ret = xSemaphoreGive((SemaphoreHandle_t)sem);
    return (ret == pdTRUE) ? H_OK : H_FAIL;
}

static int h_sem_give_from_isr_adapter(h_semaphore_t sem, void *isr_ctx)
{
    BaseType_t *task_woken = (BaseType_t *)isr_ctx;
    BaseType_t ret = xSemaphoreGiveFromISR((SemaphoreHandle_t)sem, task_woken);
    return (ret == pdTRUE) ? H_OK : H_FAIL;
}

static int h_sem_delete_adapter(h_semaphore_t sem)
{
    vSemaphoreDelete((SemaphoreHandle_t)sem);
    return H_OK;
}

/* ──  Critical Section ──
 * vPortEnterCritical / vPortExitCritical are the FreeRTOS direct port functions.
 * These disable interrupts (portDISABLE_INTERRUPTS / portENABLE_INTERRUPTS)
 * without taking the task scheduler lock. Safe for ISR context.
 * They do NOT nest via counters on SMP targets — use with caution. */

static void h_enter_critical_adapter(void)
{
    vPortEnterCritical();
}

static void h_exit_critical_adapter(void)
{
    vPortExitCritical();
}

/* ──  Timers (esp_timer adapter) ── */

typedef struct {
    esp_timer_handle_t handle;
    bool started;
} h_timer_adapter_t;

static int h_timer_create_adapter(const char *name, h_timer_t *out)
{
    (void)name;
    h_timer_adapter_t *t = calloc(1, sizeof(h_timer_adapter_t));
    if (!t) return H_ERR_NO_MEM;
    *out = (h_timer_t)t;
    return H_OK;
}

static int h_timer_start_adapter(h_timer_t t, uint32_t period_ms,
                                 bool periodic, void (*cb)(void*), void *arg)
{
    if (!t || !cb) return H_ERR_INVALID_ARG;
    h_timer_adapter_t *timer = (h_timer_adapter_t *)t;

    if (timer->started && timer->handle) {
        esp_timer_stop(timer->handle);
        esp_timer_delete(timer->handle);
        timer->handle = NULL;
        timer->started = false;
    }

    esp_timer_create_args_t args = {
        .callback = cb,
        .arg = arg,
        .dispatch_method = ESP_TIMER_TASK,
        .name = "h_timer",
    };
    if (esp_timer_create(&args, &timer->handle) != ESP_OK)
        return H_FAIL;

    esp_err_t err = periodic
        ? esp_timer_start_periodic(timer->handle, period_ms * 1000ULL)
        : esp_timer_start_once(timer->handle, period_ms * 1000ULL);
    if (err != ESP_OK) {
        esp_timer_delete(timer->handle);
        timer->handle = NULL;
        return H_FAIL;
    }

    timer->started = true;
    return H_OK;
}

static int h_timer_stop_adapter(h_timer_t t)
{
    if (!t) return H_ERR_INVALID_ARG;
    h_timer_adapter_t *timer = (h_timer_adapter_t *)t;
    if (!timer->started || !timer->handle) goto free_wrapper;
    esp_timer_stop(timer->handle);
    esp_timer_delete(timer->handle);
    timer->handle = NULL;
    timer->started = false;
free_wrapper:
    free(timer);
    return H_OK;
}

static int h_timer_delete_adapter(h_timer_t t)
{
    if (!t) return H_ERR_INVALID_ARG;
    h_timer_adapter_t *timer = (h_timer_adapter_t *)t;
    if (timer->handle) {
        if (timer->started)
            esp_timer_stop(timer->handle);
        esp_timer_delete(timer->handle);
    }
    free(timer);
    return H_OK;
}

/* ──  Time ── */

static uint64_t h_get_time_ms_adapter(void)
{
    /* esp_timer_get_time returns microseconds */
    return (uint64_t)(esp_timer_get_time() / 1000ULL);
}

static void h_msleep_adapter(uint32_t ms)
{
    vTaskDelay(pdMS_TO_TICKS(ms));
}

static void h_usleep_adapter(uint32_t us)
{
    /* Busy-wait for microsecond delays. For longer delays (>~1ms),
     * use msleep instead to avoid wasting CPU. */
    uint64_t start = esp_timer_get_time();
    while ((esp_timer_get_time() - start) < (uint64_t)us) {
        /* spin */
    }
}

static void h_blocking_delay_adapter(unsigned int iterations)
{
    volatile unsigned int i;
    for (i = 0; i < iterations; i++) {
        __asm__ __volatile__("nop");
    }
}

/* ──  Logging ── */

static void h_log_write_adapter(int level, const char *tag,
                                const char *fmt, ...)
{
    esp_log_level_t esp_level;
    switch (level) {
        case 0:  esp_level = ESP_LOG_NONE;    break;
        case 1:  esp_level = ESP_LOG_ERROR;   break;
        case 2:  esp_level = ESP_LOG_WARN;    break;
        case 3:  esp_level = ESP_LOG_INFO;    break;
        case 4:  esp_level = ESP_LOG_DEBUG;   break;
        case 5:  esp_level = ESP_LOG_VERBOSE; break;
        default: esp_level = ESP_LOG_NONE;    break;
    }

    va_list args;
    va_start(args, fmt);
    esp_log_writev(esp_level, tag, fmt, args);
    va_end(args);
}

/* ──  Optional OSAL Extension Adapters ── */

static int h_restart_host_adapter(void)
{
    /* Preserve legacy shutdown-cleanup behaviour from
     * port_esp_hosted_host_os.c:hosted_restart_host() */
    esp_unregister_shutdown_handler((shutdown_handler_t)esp_wifi_stop);
    esp_restart();
    return 0;  /* never reached */
}

static void h_hosted_init_hook_adapter(void)
{
    /* Intentionally empty — port-specific init done in h_port_init() */
}

static int h_spi_hd_set_data_lines_adapter(uint32_t data_lines)
{
    return hosted_spi_hd_set_data_lines(data_lines);
}

/* ──  Power-Save Adapters ── */

static int h_get_wakeup_reason_adapter(void)
{
#if H_HOST_PS_ALLOWED
    esp_reset_reason_t reason = esp_reset_reason();
    if (reason == ESP_RST_POWERON) {
        return HOSTED_WAKEUP_NORMAL_REBOOT;
    }
    if (reason == ESP_RST_DEEPSLEEP) {
        bool gpio_wakeup = (esp_sleep_get_wakeup_cause() == ESP_SLEEP_WAKEUP_GPIO);
        if (gpio_wakeup)
            return HOSTED_WAKEUP_DEEP_SLEEP;
    }
    return HOSTED_WAKEUP_UNDEFINED;
#else
    return HOSTED_WAKEUP_NORMAL_REBOOT;
#endif
}

static int h_config_power_save_adapter(int type, int wakeup_pin, int wakeup_level)
{
#if H_HOST_PS_ALLOWED
    if (type == HOSTED_POWER_SAVE_TYPE_DEEP_SLEEP) {
        if (!esp_sleep_is_valid_wakeup_gpio(wakeup_pin))
            return H_FAIL;
        return (esp_deep_sleep_enable_gpio_wakeup(BIT(wakeup_pin), wakeup_level) == ESP_OK)
            ? H_OK : H_FAIL;
    }
#endif
    return H_FAIL;
}

static int h_start_power_save_adapter(int type)
{
#if H_HOST_PS_ALLOWED
    if (type == HOSTED_POWER_SAVE_TYPE_DEEP_SLEEP) {
        esp_deep_sleep_start();
        return H_OK;
    }
#endif
    return H_FAIL;
}

/* ──  Global OSAL Contract Instance ── */

extern int esp_hosted_power_save_init(void);
extern int esp_hosted_woke_from_power_save(void);

const h_osal_contract_t g_h_osal = {
    /* Memory */
    .malloc            = malloc,
    .calloc            = calloc,
    .realloc           = realloc,
    .free              = free,
    .memcpy            = memcpy,
    .memset            = memset,
    .malloc_align      = h_malloc_align_adapter,
    .free_align        = free,

    /* Threads */
    .thread_create     = h_thread_create_adapter,
    .thread_delete     = h_thread_delete_adapter,

    /* Mutex */
    .mutex_create      = h_mutex_create_adapter,
    .mutex_lock        = h_mutex_lock_adapter,
    .mutex_unlock      = h_mutex_unlock_adapter,
    .mutex_delete      = h_mutex_delete_adapter,

    /* Queue */
    .queue_create      = h_queue_create_adapter,
    .queue_send        = h_queue_send_adapter,
    .queue_recv        = h_queue_recv_adapter,
    .queue_msg_waiting = h_queue_msg_waiting_adapter,
    .queue_reset       = h_queue_reset_adapter,
    .queue_delete      = h_queue_delete_adapter,

    /* Semaphore */
    .sem_create        = h_sem_create_adapter,
    .sem_take          = h_sem_take_adapter,
    .sem_give          = h_sem_give_adapter,
    .sem_give_from_isr = h_sem_give_from_isr_adapter,
    .sem_delete        = h_sem_delete_adapter,

    /* Critical Section */
    .enter_critical    = h_enter_critical_adapter,
    .exit_critical     = h_exit_critical_adapter,

    /* Timer (esp_timer adapter) */
    .timer_create      = h_timer_create_adapter,
    .timer_start       = h_timer_start_adapter,
    .timer_stop        = h_timer_stop_adapter,
    .timer_delete      = h_timer_delete_adapter,
    .get_time_ms       = h_get_time_ms_adapter,

    /* Time / Delay */
    .msleep            = h_msleep_adapter,
    .usleep            = h_usleep_adapter,
    .blocking_delay    = h_blocking_delay_adapter,

    /* Logging */
    .log_write         = h_log_write_adapter,

    /* Optional extensions (bridged to legacy port during transition) */
    .restart_host      = h_restart_host_adapter,
    .hosted_init_hook  = h_hosted_init_hook_adapter,
    .woke_from_ps      = esp_hosted_woke_from_power_save,
    .ps_init           = esp_hosted_power_save_init,
    .spi_hd_set_data_lines = h_spi_hd_set_data_lines_adapter,

    /* Power-save extensions */
    .get_host_wakeup_or_reboot_reason = h_get_wakeup_reason_adapter,
    .config_host_power_save_hal       = h_config_power_save_adapter,
    .start_host_power_save_hal        = h_start_power_save_adapter,
};
