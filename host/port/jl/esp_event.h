/* host/port/jl/esp_event.h
 * JL shadow of ESP-IDF esp_event.h.
 *
 * Provides the minimal event-base declarations used by host/esp_hosted_event.h.
 */

#ifndef JL_ESP_EVENT_H
#define JL_ESP_EVENT_H

#include <stdint.h>

typedef const char *esp_event_base_t;

#define ESP_EVENT_DECLARE_BASE(id) extern esp_event_base_t id
#define ESP_EVENT_DEFINE_BASE(id)  esp_event_base_t id = #id

#endif /* JL_ESP_EVENT_H */
