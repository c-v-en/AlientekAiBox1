/*
 * GPIO LED Driver Implementation
 */

#include "led_gpio.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include <stdlib.h>

static const char *TAG = "led_gpio";

static esp_err_t gpio_led_init(led_device_t *dev) {
    if (!dev || !dev->driver_ctx) return ESP_ERR_INVALID_ARG;
    
    led_gpio_ctx_t *ctx = (led_gpio_ctx_t *)dev->driver_ctx;
    
    gpio_config_t cfg = {
        .pin_bit_mask = (1ULL << ctx->gpio_num),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    
    esp_err_t ret = gpio_config(&cfg);
    if (ret != ESP_OK) return ret;
    
    /* 初始状态：熄灭 */
    if (ctx->active_low) {
        gpio_set_level(ctx->gpio_num, 1);  // 高电平=灭
    } else {
        gpio_set_level(ctx->gpio_num, 0);  // 低电平=灭
    }
    
    dev->is_on = false;
    dev->initialized = true;
    
    ESP_LOGI(TAG, "GPIO%d LED initialized (%s)", 
             ctx->gpio_num, ctx->active_low ? "active low" : "active high");
    return ESP_OK;
}

static esp_err_t gpio_led_deinit(led_device_t *dev) {
    (void)dev;
    return ESP_OK;
}

static esp_err_t gpio_led_on(led_device_t *dev) {
    if (!dev || !dev->driver_ctx) return ESP_ERR_INVALID_ARG;
    
    led_gpio_ctx_t *ctx = (led_gpio_ctx_t *)dev->driver_ctx;
    
    if (ctx->active_low) {
        gpio_set_level(ctx->gpio_num, 0);  // 低电平=亮
    } else {
        gpio_set_level(ctx->gpio_num, 1);  // 高电平=亮
    }
    
    return ESP_OK;
}

static esp_err_t gpio_led_off(led_device_t *dev) {
    if (!dev || !dev->driver_ctx) return ESP_ERR_INVALID_ARG;
    
    led_gpio_ctx_t *ctx = (led_gpio_ctx_t *)dev->driver_ctx;
    
    if (ctx->active_low) {
        gpio_set_level(ctx->gpio_num, 1);  // 高电平=灭
    } else {
        gpio_set_level(ctx->gpio_num, 0);  // 低电平=灭
    }
    
    return ESP_OK;
}

static esp_err_t gpio_led_toggle(led_device_t *dev) {
    if (!dev) return ESP_ERR_INVALID_ARG;
    
    if (dev->is_on) {
        return gpio_led_off(dev);
    } else {
        return gpio_led_on(dev);
    }
}

static bool gpio_led_is_on(led_device_t *dev) {
    if (!dev) return false;
    return dev->is_on;
}

/* 驱动接口实例 */
const led_driver_iface_t led_gpio_driver = {
    .name = "gpio",
    .init = gpio_led_init,
    .deinit = gpio_led_deinit,
    .on = gpio_led_on,
    .off = gpio_led_off,
    .toggle = gpio_led_toggle,
    .is_on = gpio_led_is_on,
};

/* 创建上下文 */
led_gpio_ctx_t* led_gpio_create_ctx(int gpio_num, bool active_low) {
    led_gpio_ctx_t *ctx = malloc(sizeof(led_gpio_ctx_t));
    if (!ctx) return NULL;
    
    ctx->gpio_num = gpio_num;
    ctx->active_low = active_low;
    
    return ctx;
}

/* LED设备管理函数（放在这里避免单独创建led_driver.c）*/
#include "led_driver.h"

led_device_t* led_device_create(const led_driver_iface_t *driver, void *driver_ctx) {
    if (!driver) return NULL;
    
    led_device_t *dev = malloc(sizeof(led_device_t));
    if (!dev) return NULL;
    
    dev->driver = driver;
    dev->driver_ctx = driver_ctx;
    dev->is_on = false;
    dev->initialized = false;
    
    return dev;
}

void led_device_destroy(led_device_t *dev) {
    if (dev) {
        if (dev->driver && dev->driver->deinit) {
            dev->driver->deinit(dev);
        }
        free(dev);
    }
}
