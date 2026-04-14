/*
 * SmartConfig Component
 * 面向对象设计的配网组件，支持依赖注入和状态观察
 */

#ifndef SMARTCONFIG_COMPONENT_H
#define SMARTCONFIG_COMPONENT_H

#include <stdint.h>
#include <stdbool.h>
#include "esp_err.h"
#include "esp_wifi.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "freertos/semphr.h"
#include "freertos/timers.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ==================== 类型定义 ==================== */

/**
 * @brief 组件最大观察者数量
 */
#define SC_MAX_OBSERVERS 5

/**
 * @brief SmartConfig 状态
 */
typedef enum {
    SC_STATUS_IDLE = 0,
    SC_STATUS_STARTING,
    SC_STATUS_SCANNING,
    SC_STATUS_FOUND_CHANNEL,
    SC_STATUS_GETTING_SSID_PSWD,
    SC_STATUS_CONNECTING,
    SC_STATUS_CONNECTED,
    SC_STATUS_FAILED,
    SC_STATUS_STOPPED,
} sc_status_t;

/**
 * @brief WiFi 连接信息
 */
typedef struct {
    char ssid[33];
    char password[65];
    uint8_t bssid[6];
    uint8_t channel;
} sc_wifi_info_t;

/* 前向声明对象结构体 */
typedef struct sc_component sc_component_t;

/**
 * @brief 状态回调函数类型
 */
typedef void (*sc_status_callback_t)(sc_component_t *component,
                                      sc_status_t status,
                                      const sc_wifi_info_t *info,
                                      void *user_data);

/* ==================== 抽象接口定义 ==================== */

/**
 * @brief SmartConfig 驱动接口
 * 支持 ESPTOUCH、AirKiss 等多种配网协议
 */
typedef struct sc_driver_iface {
    const char *name;
    
    /**
     * @brief 初始化驱动
     */
    esp_err_t (*init)(void *ctx);
    
    /**
     * @brief 反初始化驱动
     */
    esp_err_t (*deinit)(void *ctx);
    
    /**
     * @brief 启动配网
     * @param timeout_ms 超时时间(毫秒)
     */
    esp_err_t (*start)(void *ctx, uint32_t timeout_ms);
    
    /**
     * @brief 停止配网
     */
    esp_err_t (*stop)(void *ctx);
    
    /**
     * @brief 获取当前状态
     */
    sc_status_t (*get_status)(void *ctx);
    
} sc_driver_iface_t;

/**
 * @brief 存储接口
 * 支持 NVS、文件系统等多种存储后端
 */
typedef struct sc_storage_iface {
    /**
     * @brief 初始化存储
     */
    esp_err_t (*init)(void *ctx);
    
    /**
     * @brief 反初始化存储
     */
    esp_err_t (*deinit)(void *ctx);
    
    /**
     * @brief 加载WiFi配置
     */
    esp_err_t (*load_wifi_config)(void *ctx, wifi_config_t *config);
    
    /**
     * @brief 保存WiFi配置
     */
    esp_err_t (*save_wifi_config)(void *ctx, const wifi_config_t *config);
    
    /**
     * @brief 清除WiFi配置
     */
    esp_err_t (*clear_wifi_config)(void *ctx);
    
    /**
     * @brief 检查是否有保存的配置
     */
    bool (*has_config)(void *ctx);
    
} sc_storage_iface_t;

/**
 * @brief WiFi管理接口
 * 抽象WiFi操作，便于移植和测试
 */
typedef struct sc_wifi_iface {
    /**
     * @brief 初始化WiFi
     */
    esp_err_t (*init)(void *ctx);
    
    /**
     * @brief 反初始化WiFi
     */
    esp_err_t (*deinit)(void *ctx);
    
    /**
     * @brief 连接到指定AP
     */
    esp_err_t (*connect)(void *ctx, const wifi_config_t *config);
    
    /**
     * @brief 断开连接
     */
    esp_err_t (*disconnect)(void *ctx);
    
    /**
     * @brief 检查是否已连接
     */
    bool (*is_connected)(void *ctx);
    
} sc_wifi_iface_t;

/**
 * @brief 观察者结构
 */
typedef struct sc_observer {
    sc_status_callback_t callback;
    void *user_data;
    bool active;
} sc_observer_t;

/**
 * @brief 组件配置
 */
typedef struct {
    uint32_t task_stack_size;
    uint32_t task_priority;
    bool auto_save_wifi;
    bool auto_reconnect;
} sc_config_t;

/* ==================== 对象结构体 ==================== */

/**
 * @brief SmartConfig组件对象
 * 封装所有状态和依赖
 */
struct sc_component {
    /* 对象标识 */
    const char *name;
    uint32_t instance_id;
    
    /* 依赖注入：策略模式 */
    const sc_driver_iface_t *driver;
    const sc_storage_iface_t *storage;
    const sc_wifi_iface_t *wifi;
    
    /* 各模块上下文 */
    void *driver_ctx;
    void *storage_ctx;
    void *wifi_ctx;
    
    /* 内部状态（完全封装） */
    sc_status_t status;
    sc_wifi_info_t wifi_info;
    wifi_config_t saved_config;
    bool has_saved_wifi;
    
    /* FreeRTOS资源 */
    EventGroupHandle_t event_group;
    SemaphoreHandle_t mutex;
    TaskHandle_t task;
    TimerHandle_t timer;
    
    /* 观察者列表 */
    sc_observer_t observers[SC_MAX_OBSERVERS];
    
    /* 配置 */
    sc_config_t config;
    
    /* 运行标志 */
    bool is_running;
    bool is_initialized;
};

/* ==================== 类方法（API） ==================== */

/**
 * @brief 构造函数 - 创建组件实例
 * 
 * @param name 实例名称
 * @param driver 配网驱动接口
 * @param driver_ctx 驱动上下文
 * @param storage 存储接口
 * @param storage_ctx 存储上下文
 * @param wifi WiFi管理接口
 * @param wifi_ctx WiFi上下文
 * @param config 组件配置（可为NULL，使用默认配置）
 * @return sc_component_t* 组件实例指针
 */
sc_component_t* sc_component_create(const char *name,
                                     const sc_driver_iface_t *driver,
                                     void *driver_ctx,
                                     const sc_storage_iface_t *storage,
                                     void *storage_ctx,
                                     const sc_wifi_iface_t *wifi,
                                     void *wifi_ctx,
                                     const sc_config_t *config);

/**
 * @brief 析构函数 - 销毁组件实例
 */
void sc_component_destroy(sc_component_t *self);

/**
 * @brief 初始化组件
 */
esp_err_t sc_component_init(sc_component_t *self);

/**
 * @brief 反初始化组件
 */
esp_err_t sc_component_deinit(sc_component_t *self);

/**
 * @brief 启动配网流程
 * @param timeout_ms 超时时间(毫秒)，0表示无超时
 */
esp_err_t sc_component_start(sc_component_t *self, uint32_t timeout_ms);

/**
 * @brief 停止配网流程
 */
esp_err_t sc_component_stop(sc_component_t *self);

/**
 * @brief 获取当前状态
 */
sc_status_t sc_component_get_status(sc_component_t *self);

/**
 * @brief 注册状态观察者
 */
esp_err_t sc_component_attach_observer(sc_component_t *self, 
                                        sc_status_callback_t callback,
                                        void *user_data);

/**
 * @brief 注销状态观察者
 */
esp_err_t sc_component_detach_observer(sc_component_t *self,
                                        sc_status_callback_t callback);

/**
 * @brief 检查是否有保存的WiFi配置
 */
bool sc_component_has_saved_wifi(sc_component_t *self);

/**
 * @brief 获取保存的WiFi信息
 */
esp_err_t sc_component_get_saved_wifi(sc_component_t *self, 
                                       sc_wifi_info_t *info);

/**
 * @brief 清除保存的WiFi配置
 */
esp_err_t sc_component_clear_wifi(sc_component_t *self);

/**
 * @brief 使用保存的配置连接WiFi（开机自动连接）
 */
esp_err_t sc_component_connect_saved(sc_component_t *self);

/**
 * @brief 获取默认配置
 */
sc_config_t sc_component_get_default_config(void);

/* ==================== 内置驱动声明 ==================== */

/**
 * @brief ESPTOUCH驱动接口
 */
extern const sc_driver_iface_t sc_driver_esptouch;

/**
 * @brief NVS存储接口
 */
extern const sc_storage_iface_t sc_storage_nvs;

/**
 * @brief ESP32 WiFi管理接口
 */
extern const sc_wifi_iface_t sc_wifi_esp32;

#ifdef __cplusplus
}
#endif

#endif /* SMARTCONFIG_COMPONENT_H */
