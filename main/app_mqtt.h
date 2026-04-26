/*
 * MQTT 客户端管理器
 * 面向阿里云私有部署 EMQX (TLS)
 * 运行于 Core0 网络域，通过事件总线与 Core1 交互
 */

#ifndef APP_MQTT_H
#define APP_MQTT_H

#include "esp_err.h"
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    APP_MQTT_STATE_DISCONNECTED = 0,
    APP_MQTT_STATE_CONNECTING,
    APP_MQTT_STATE_CONNECTED,
    APP_MQTT_STATE_ERROR,
} app_mqtt_state_t;

/**
 * @brief 初始化 MQTT 客户端（不启动连接）
 */
esp_err_t app_mqtt_init(void);

/**
 * @brief 启动 MQTT 连接（建议在 WiFi 就绪后调用）
 */
esp_err_t app_mqtt_start(void);

/**
 * @brief 停止 MQTT 连接
 */
esp_err_t app_mqtt_stop(void);

/**
 * @brief 获取当前 MQTT 状态
 */
app_mqtt_state_t app_mqtt_get_state(void);

/**
 * @brief 发布 MQTT 消息（非阻塞，可从任意核调用）
 * @param topic 主题
 * @param data 数据指针
 * @param len 数据长度
 * @param qos QoS 等级 (0/1/2)
 * @param retain 是否保留消息
 * @return ESP_OK 成功入队
 */
esp_err_t app_mqtt_publish(const char *topic, const char *data, size_t len, int qos, int retain);

/**
 * @brief 订阅主题（非阻塞，可从任意核调用）
 * @param topic 主题
 * @param qos QoS 等级
 * @return ESP_OK 成功入队
 */
esp_err_t app_mqtt_subscribe(const char *topic, int qos);

#ifdef __cplusplus
}
#endif

#endif /* APP_MQTT_H */
