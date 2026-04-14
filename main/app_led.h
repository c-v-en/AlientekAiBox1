/*
 * Application LED Manager
 * 应用层LED管理器（GPIO4系统状态指示）
 */

#ifndef APP_LED_H
#define APP_LED_H

#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief LED系统状态
 */
typedef enum {
    LED_SYS_IDLE = 0,           // 空闲/已连接：GPIO4(1Hz)
    LED_SYS_PROVISIONING,       // 配网中：GPIO4(5Hz)
    LED_SYS_PROV_TIMEOUT,       // 配网超时：GPIO4(1Hz)
} led_system_state_t;

/**
 * @brief 初始化LED管理器（创建设备，不启动任务）
 */
esp_err_t app_led_init(void);

/**
 * @brief 启动LED任务（优先级1，最低）
 * 应在所有外设初始化完成后调用
 */
esp_err_t app_led_start(void);

/**
 * @brief 设置LED系统状态
 * 自动根据状态控制LED
 */
void app_led_set_state(led_system_state_t state);

/**
 * @brief 获取当前状态
 */
led_system_state_t app_led_get_state(void);

#ifdef __cplusplus
}
#endif

#endif /* APP_LED_H */
