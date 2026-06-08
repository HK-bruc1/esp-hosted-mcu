/*
 * SPDX-FileCopyrightText: 2025-2026 Espressif Systems (Shanghai) CO LTD
 * SPDX-License-Identifier: Apache-2.0
 */

#include "h_transport_gpio.h"
#include "h_port_config.h"

#include <driver/gpio.h>
#include <esp_log.h>

#define TAG "h_gpio"

int espidf_gpio_config(uint32_t gpio_num, uint32_t mode)
{
    gpio_config_t io_conf = {
        .intr_type    = GPIO_INTR_DISABLE,
        .mode         = mode,
        .pin_bit_mask = (1ULL << gpio_num),
        .pull_down_en = 0,
        .pull_up_en   = 0,
    };
    ESP_LOGI(TAG, "GPIO [%d] configured", (int)gpio_num);
    gpio_config(&io_conf);
    return 0;
}

int espidf_gpio_setup_intr(uint32_t gpio_num, uint32_t intr_type,
                      void (*fn)(void *), void *arg)
{
    static bool isr_service_installed = false;

    gpio_config_t new_gpio_io_conf = {
        .mode         = GPIO_MODE_INPUT,
        .intr_type    = GPIO_INTR_DISABLE,
        .pin_bit_mask = (1ULL << gpio_num),
    };

    if (intr_type == H_GPIO_INTR_NEGEDGE) {
        new_gpio_io_conf.pull_up_en = 1;
    } else {
        new_gpio_io_conf.pull_down_en = 1;
    }

    ESP_LOGI(TAG, "GPIO [%d] configuring as Interrupt", (int)gpio_num);
    ESP_ERROR_CHECK_WITHOUT_ABORT(gpio_config(&new_gpio_io_conf));

    if (!isr_service_installed) {
        gpio_install_isr_service(0);
        isr_service_installed = true;
    }

    gpio_isr_handler_remove(gpio_num);
    ESP_ERROR_CHECK_WITHOUT_ABORT(gpio_isr_handler_add(gpio_num, fn, arg));

    int ret = gpio_set_intr_type(gpio_num, intr_type);
    if (ret != ESP_OK) {
        gpio_isr_handler_remove(gpio_num);
        return ret;
    }

    ESP_ERROR_CHECK_WITHOUT_ABORT(gpio_intr_enable(gpio_num));
    return ret;
}

int espidf_gpio_clear_intr(uint32_t gpio_num)
{
    gpio_intr_disable(gpio_num);
    return gpio_isr_handler_remove(gpio_num);
}

int espidf_gpio_read(uint32_t gpio_num)
{
    return gpio_get_level(gpio_num);
}

int espidf_gpio_write(uint32_t gpio_num, uint32_t value)
{
    return gpio_set_level(gpio_num, value);
}

int espidf_gpio_pull(uint32_t gpio_num, uint32_t pull_value, uint32_t enable)
{
    if (pull_value == H_GPIO_PULL_UP) {
        return enable ? gpio_pullup_en(gpio_num) : gpio_pullup_dis(gpio_num);
    } else {
        return enable ? gpio_pulldown_en(gpio_num) : gpio_pulldown_dis(gpio_num);
    }
}

int espidf_gpio_hold(uint32_t gpio_num, uint32_t hold_value)
{
    if (hold_value) {
        return gpio_hold_en(gpio_num);
    } else {
        return gpio_hold_dis(gpio_num);
    }
}
