/*
 * SPDX-FileCopyrightText: 2026 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef __ESP_HOSTED_MISC_TYPES_H__
#define __ESP_HOSTED_MISC_TYPES_H__

#include <stdbool.h>
#include <stdint.h>

typedef struct {
	uint32_t magic_word;        /*!< Magic word ESP_HOSTED_APP_DESC_MAGIC_WORD */
	uint32_t secure_version;    /*!< Secure version */
	uint32_t reserv1[2];        /*!< reserv1 */
	char version[32];           /*!< Application version */
	char project_name[32];      /*!< Project name */
	char time[16];              /*!< Compile time */
	char date[16];              /*!< Compile date*/
	char idf_ver[32];           /*!< Version IDF */
	uint8_t app_elf_sha256[32]; /*!< sha256 of elf file */
	uint16_t min_efuse_blk_rev_full; /*!< Minimal eFuse block revision supported by image, in format: major * 100 + minor */
	uint16_t max_efuse_blk_rev_full; /*!< Maximal eFuse block revision supported by image, in format: major * 100 + minor */
	uint8_t mmu_page_size;      /*!< MMU page size in log base 2 format */
	uint8_t reserv3[3];         /*!< reserv3 */
	uint32_t reserv2[18];       /*!< reserv2 */
} esp_hosted_app_desc_t;

typedef enum {
	ESP_HOSTED_MEMMONITOR_NO_CHANGE = 0, // don't change the monitor configuration. Used to query current heap values
	ESP_HOSTED_MEMMONITOR_DISABLE = 1,   // disable the monitor
	ESP_HOSTED_MEMMONITOR_ENABLE = 2,    // (re)enable the monitor with new configuration
} esp_hosted_mem_monitor_config_t;

/**
 * @brief Structures related to memory monitor
 */
typedef struct {
	uint32_t threshold_mem_dma;  /*!< heap memory threshold for DMA capable memory */
	uint32_t threshold_mem_8bit; /*!< heap memory threshold for 8/16...-bit capable memory */
} esp_hosted_mem_monitor_threshold_t;

typedef struct {
	uint32_t free_size;          /*!< free size of heap */
	uint32_t largest_free_block; /*!< largest free block that can be allocated from the heap */
} esp_hosted_mem_info_t;

typedef struct {
	esp_hosted_mem_info_t cap_dma;  /*!< DMA capable memory */
	esp_hosted_mem_info_t cap_8bit; /*!< 8/16...-bit capable memory */
} esp_hosted_cap_info_t;

// structure to configure memory monitoring
typedef struct {
	esp_hosted_mem_monitor_config_t config;          /*!< configure the memory monitor */
	bool report_always;                              /*!< if true, a memory report event is sent every interval_sec */
	uint32_t interval_sec;                           /*!< interval between memory checks on the co-processor and sending a report */
	esp_hosted_mem_monitor_threshold_t internal_mem; /*!< thresholds for internal memory */
	esp_hosted_mem_monitor_threshold_t external_mem; /*!< thresholds for external memory */
} esp_hosted_config_mem_monitor_t;

// current memory info returned in response to configure memory monitoring
typedef struct {
	esp_hosted_mem_monitor_config_t config; /*!< current memory monitor configuration */
	bool report_always;                     /*!< if true, a memory report event is sent every interval_sec */
	uint32_t interval_sec;                  /*!< interval between memory checks on the co-processor and sending a report */
	uint32_t curr_total_heap_size;          /*!< current total heap size on co-processor */
	esp_hosted_cap_info_t curr_internal;    /*!< current internal heap sizes */
	esp_hosted_cap_info_t curr_external;    /*!< current external heap sized */
} esp_hosted_curr_mem_info_t;

// memory monitoring event info
typedef struct {
	uint32_t curr_total_free_heap_size;  /*!< current total free heap size */
	uint32_t curr_min_free_heap_size;    /*!< current minimum free heap size since boot up */
	esp_hosted_cap_info_t curr_internal; /*!< current internal heap sizes */
	esp_hosted_cap_info_t curr_external; /*!< current external heap sizes */
} esp_hosted_event_mem_info_t;

#endif
