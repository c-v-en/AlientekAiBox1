/*
 * LED Driver Abstraction Interface
 * 面向对象的LED驱动抽象接口
 */

#ifndef LED_DRIVER_H
#define LED_DRIVER_H

#include "esp_err.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* 前向声明 */
typedef struct led_device led_device_t;

/**
 * @brief LED驱动接口（类似C++虚函数表）
 */
typedef struct led_driver_iface {
    const char *name;
    
    esp_err_t (*init)(led_device_t *dev);
    esp_err_t (*deinit)(led_device_t *dev);
    
    esp_err_t (*on)(led_device_t *dev);
    esp_err_t (*off)(led_device_t *dev);
    esp_err_t (*toggle)(led_device_t *dev);
    
    bool (*is_on)(led_device_t *dev);
} led_driver_iface_t;

/**
 * @brief LED设备实例
 */
struct led_device {
    const led_driver_iface_t *driver;
    void *driver_ctx;
    bool is_on;
    bool initialized;
};

/* ==================== 设备管理 API ==================== */

/**
 * @brief 创建设备实例
 */
led_device_t* led_device_create(const led_driver_iface_t *driver, void *driver_ctx);

/**
 * @brief 销毁设备实例
 */
void led_device_destroy(led_device_t *dev);

/**
 * @brief 初始化设备
 */
static inline esp_err_t led_init(led_device_t *dev) {
    if (!dev || !dev->driver) return ESP_ERR_INVALID_ARG;
    return dev->driver->init(dev);
}

/**
 * @brief 点亮LED
 */
static inline esp_err_t led_on(led_device_t *dev) {
    if (!dev || !dev->driver || !dev->driver->on) return ESP_ERR_INVALID_ARG;
    esp_err_t ret = dev->driver->on(dev);
    if (ret == ESP_OK) dev->is_on = true;
    return ret;
}

/**
 * @brief 熄灭LED
 */
static inline esp_err_t led_off(led_device_t *dev) {
    if (!dev || !dev->driver || !dev->driver->off) return ESP_ERR_INVALID_ARG;
    esp_err_t ret = dev->driver->off(dev);
    if (ret == ESP_OK) dev->is_on = false;
    return ret;
}

/**
 * @brief 翻转LED状态
 */
static inline esp_err_t led_toggle(led_device_t *dev) {
    if (!dev || !dev->driver || !dev->driver->toggle) return ESP_ERR_INVALID_ARG;
    return dev->driver->toggle(dev);
}

/**
 * @brief 获取LED状态
 */
static inline bool led_is_on(led_device_t *dev) {
    if (!dev) return false;
    return dev->is_on;
}

#ifdef __cplusplus
}
#endif

#endif /* LED_DRIVER_H */
