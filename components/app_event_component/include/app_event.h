/*
 * 事件分发层头文件
 * 支持优先级的事件分发系统
 */

#ifndef APP_EVENT_H
#define APP_EVENT_H

#include <stdint.h>
#include <stdbool.h>
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ==================== 事件类型 ==================== */

typedef enum {
    /* WiFi事件 (高优先级 3-4) */
    APP_EVENT_WIFI_IDLE = 0,
    APP_EVENT_WIFI_STARTING,        /* 优先级: 3 */
    APP_EVENT_WIFI_SCANNING,        /* 优先级: 3 */
    APP_EVENT_WIFI_CONNECTING,      /* 优先级: 4 */
    APP_EVENT_WIFI_CONNECTED,       /* 优先级: 4 */
    APP_EVENT_WIFI_DISCONNECTED,    /* 优先级: 4 */
    APP_EVENT_WIFI_FAILED,          /* 优先级: 3 */
    
    /* 配网事件 (中优先级 2) */
    APP_EVENT_PROV_START,           /* 优先级: 2 */
    APP_EVENT_PROV_SUCCESS,         /* 优先级: 2 */
    APP_EVENT_PROV_TIMEOUT,         /* 优先级: 2 */
    APP_EVENT_PROV_STOPPED,         /* 优先级: 2 */
    
    /* 按键事件 (高优先级 3) */
    APP_EVENT_KEY0_SHORT,           /* KEY0短按 */
    APP_EVENT_KEY0_LONG,            /* KEY0长按 */
    APP_EVENT_KEY0_DOUBLE,          /* KEY0双击 */
    APP_EVENT_KEY1_SHORT,           /* KEY1短按 */
    APP_EVENT_KEY1_LONG,            /* KEY1长按 */
    APP_EVENT_KEY1_DOUBLE,          /* KEY1双击 */
    
    /* MQTT 事件 (高优先级 3) */
    APP_EVENT_MQTT_CONNECTED,       /* MQTT 已连接 */
    APP_EVENT_MQTT_DISCONNECTED,    /* MQTT 断开 */
    APP_EVENT_MQTT_DATA,            /* 收到 MQTT 消息 */
    
    /* 系统事件 (低优先级 1) */
    APP_EVENT_INIT_DONE,            /* 优先级: 1 */
    APP_EVENT_VERSION_INFO,         /* 版本信息已就绪: 1 */
    
    APP_EVENT_MAX
} app_event_id_t;

/* ==================== 优先级定义 ==================== */

typedef enum {
    APP_EVENT_PRIO_LOW = 1,         /* 日志、状态更新 */
    APP_EVENT_PRIO_NORMAL = 2,      /* 常规外设事件 */
    APP_EVENT_PRIO_HIGH = 3,        /* WiFi状态变化 */
    APP_EVENT_PRIO_CRITICAL = 4,    /* 连接/断开等关键状态 */
} app_event_priority_t;

/* ==================== 事件数据结构 ==================== */

typedef struct {
    app_event_id_t id;
    app_event_priority_t priority;  /* 事件优先级 */
    uint32_t timestamp;             /* 发送时间戳 */
    
    union {
        struct {
            char ssid[33];
            char password[65];
            uint8_t channel;
        } wifi;
        struct {
            int error_code;
        } error;
        struct {
            char topic[48];
            char payload[80];
            uint16_t payload_len;
            uint16_t topic_len;
        } mqtt;
        struct {
            uint8_t data[128];
        } raw;
    } data;
} app_event_t;

/* 回调函数类型 */
typedef void (*app_event_handler_t)(const app_event_t *event, void *user_data);

/* ==================== API ==================== */

/**
 * @brief 初始化事件分发系统
 */
esp_err_t app_event_init(void);

/**
 * @brief 注册事件监听
 * @param event_id 监听的事件ID (-1 监听所有事件)
 * @param handler 回调函数
 * @param user_data 用户数据
 */
esp_err_t app_event_register_listener(app_event_id_t event_id,
                                       app_event_handler_t handler,
                                       void *user_data);

/**
 * @brief 注销事件监听
 */
esp_err_t app_event_unregister_listener(app_event_handler_t handler);

/**
 * @brief 发布事件（带优先级）
 * @param event_id 事件ID
 * @param priority 优先级
 * @param data 事件数据（可为NULL）
 */
esp_err_t app_event_post(app_event_id_t event_id,
                          app_event_priority_t priority,
                          const void *data,
                          size_t data_size);

/**
 * @brief 发布简单事件（默认优先级）
 */
esp_err_t app_event_post_simple(app_event_id_t event_id);

/**
 * @brief 清空事件队列
 */
void app_event_flush(void);

#ifdef __cplusplus
}
#endif

#endif /* APP_EVENT_H */
