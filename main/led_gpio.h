/*
 * GPIO LED Driver
 * GPIO控制LED驱动实现
 */

#ifndef LED_GPIO_H
#define LED_GPIO_H

#include "led_driver.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief GPIO LED驱动上下文
 */
typedef struct {
    int gpio_num;
    bool active_low;
} led_gpio_ctx_t;

/**
 * @brief GPIO LED驱动接口（全局常量）
 */
extern const led_driver_iface_t led_gpio_driver;

/**
 * @brief 创建驱动上下文
 * @param gpio_num GPIO号（如GPIO_NUM_4）
 * @param active_low true=低电平点亮，false=高电平点亮
 * @return 上下文指针（需传入led_device_create）
 */
led_gpio_ctx_t* led_gpio_create_ctx(int gpio_num, bool active_low);

#ifdef __cplusplus
}
#endif

#endif /* LED_GPIO_H */
