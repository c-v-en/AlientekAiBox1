/*
 * WiFi管理器
 * 封装SmartConfig组件，支持自动连接和配网超时管理
 */

#ifndef APP_WIFI_H
#define APP_WIFI_H

#include "esp_err.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief WiFi管理器状态
 */
typedef enum {
    APP_WIFI_STATE_IDLE = 0,
    APP_WIFI_STATE_CONNECTING,
    APP_WIFI_STATE_CONNECTED,
    APP_WIFI_STATE_PROVISIONING,    /* 正在配网 */
    APP_WIFI_STATE_PROV_TIMEOUT,    /* 配网超时，等待触发 */
    APP_WIFI_STATE_ERROR,
} app_wifi_state_t;

/* ==================== API ==================== */

/**
 * @brief 初始化WiFi管理器
 */
esp_err_t app_wifi_init(void);

/**
 * @brief 启动WiFi（根据配置自动决定连接或配网）
 * @return ESP_OK 成功
 */
esp_err_t app_wifi_start(void);

/**
 * @brief 停止WiFi/配网
 */
esp_err_t app_wifi_stop(void);

/**
 * @brief 获取当前状态
 */
app_wifi_state_t app_wifi_get_state(void);

/**
 * @brief 是否已连接
 */
bool app_wifi_is_connected(void);

/**
 * @brief 获取保存的SSID
 */
esp_err_t app_wifi_get_ssid(char *ssid, size_t len);

/**
 * @brief 手动启动配网（LCD按钮调用）
 * 仅当配网超时时有效
 * @return ESP_OK 启动成功
 *         ESP_ERR_INVALID_STATE 已在配网或已连接
 */
esp_err_t app_wifi_start_provisioning(void);

/**
 * @brief 清除WiFi配置
 */
esp_err_t app_wifi_clear_config(void);

#ifdef __cplusplus
}
#endif

#endif
