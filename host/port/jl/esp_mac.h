/* host/port/jl/esp_mac.h
 * JL shadow of ESP-IDF esp_mac.h.
 *
 * The portable h_types.h defines h_mac_type_t as the replacement for
 * esp_mac_type_t.  This shadow re-exports the name expected by
 * host/esp_hosted_misc.h and host/api/src/esp_hosted_api.c.
 */

#ifndef JL_ESP_MAC_H
#define JL_ESP_MAC_H

#include "h_types.h"

typedef h_mac_type_t esp_mac_type_t;

/* Extended MAC types referenced by esp_hosted_api.c. */
#define ESP_MAC_IEEE802154  H_MAC_BT
#define ESP_MAC_EFUSE_EXT   H_MAC_ETH

#endif /* JL_ESP_MAC_H */
