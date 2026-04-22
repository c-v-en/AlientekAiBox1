/*
 * SmartConfig Component 实现
 * 面向对象设计，封装状态和依赖
 */

#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "freertos/timers.h"
#include "esp_log.h"
#include "smartconfig_component.h"

static const char *TAG = "sc_component";

/* 内部事件位 */
#define SC_EVENT_START          BIT0
#define SC_EVENT_STOP           BIT1
#define SC_EVENT_CONNECTED      BIT2
#define SC_EVENT_FAILED         BIT3
#define SC_EVENT_TIMEOUT        BIT4
#define SC_EVENT_GOT_SSID_PSWD  BIT5

/* 实例计数器 */
static uint32_t s_instance_counter = 0;

/* ==================== 私有函数声明 ==================== */

static void sc_task(void *pvParameters);
static void sc_timer_callback(TimerHandle_t xTimer);
static void sc_notify_observers(sc_component_t *self);
static esp_err_t sc_lock(sc_component_t *self);
static void sc_unlock(sc_component_t *self);
static void sc_set_status(sc_component_t *self, sc_status_t status);

/* ==================== 构造函数/析构函数 ==================== */

sc_component_t* sc_component_create(const char *name,
                                     const sc_driver_iface_t *driver,
                                     void *driver_ctx,
                                     const sc_storage_iface_t *storage,
                                     void *storage_ctx,
                                     const sc_wifi_iface_t *wifi,
                                     void *wifi_ctx,
                                     const sc_config_t *config)
{
    if (driver == NULL || storage == NULL || wifi == NULL) {
        ESP_LOGE(TAG, "Invalid arguments: driver, storage, wifi cannot be NULL");
        return NULL;
    }

    sc_component_t *self = calloc(1, sizeof(sc_component_t));
    if (self == NULL) {
        ESP_LOGE(TAG, "Failed to allocate memory for component");
        return NULL;
    }

    /* 基础信息 */
    self->name = name ? name : "unnamed";
    self->instance_id = ++s_instance_counter;
    
    /* 依赖注入 */
    self->driver = driver;
    self->driver_ctx = driver_ctx;
    self->storage = storage;
    self->storage_ctx = storage_ctx;
    self->wifi = wifi;
    self->wifi_ctx = wifi_ctx;
    
    /* 配置 */
    if (config) {
        self->config = *config;
    } else {
        self->config = sc_component_get_default_config();
    }
    
    /* 初始状态 */
    self->status = SC_STATUS_IDLE;
    self->is_running = false;
    self->is_initialized = false;
    
    ESP_LOGI(TAG, "Component '%s' (id=%lu) created", self->name, self->instance_id);
    
    return self;
}

void sc_component_destroy(sc_component_t *self)
{
    if (self == NULL) {
        return;
    }

    /* 确保先反初始化 */
    if (self->is_initialized) {
        sc_component_deinit(self);
    }

    ESP_LOGI(TAG, "Component '%s' (id=%lu) destroyed", self->name, self->instance_id);
    
    free(self);
}

/* ==================== 初始化/反初始化 ==================== */

esp_err_t sc_component_init(sc_component_t *self)
{
    if (self == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    
    if (self->is_initialized) {
        return ESP_OK;
    }

    esp_err_t ret;
    
    /* 创建同步资源 */
    self->mutex = xSemaphoreCreateMutex();
    if (self->mutex == NULL) {
        ESP_LOGE(TAG, "Failed to create mutex");
        return ESP_FAIL;
    }
    
    self->event_group = xEventGroupCreate();
    if (self->event_group == NULL) {
        vSemaphoreDelete(self->mutex);
        self->mutex = NULL;
        ESP_LOGE(TAG, "Failed to create event group");
        return ESP_FAIL;
    }
    
    /* 初始化存储 */
    ret = self->storage->init ? self->storage->init(self->storage_ctx) : ESP_OK;
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Storage init failed: %s", esp_err_to_name(ret));
        goto fail;
    }
    
    /* 检查是否有保存的WiFi配置 */
    if (self->storage->has_config(self->storage_ctx)) {
        ret = self->storage->load_wifi_config(self->storage_ctx, &self->saved_config);
        if (ret == ESP_OK) {
            self->has_saved_wifi = true;
            memcpy(self->wifi_info.ssid, self->saved_config.sta.ssid, 32);
            self->wifi_info.ssid[32] = '\0';
        }
    }
    
    /* 初始化WiFi */
    ret = self->wifi->init(self->wifi_ctx);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "WiFi init failed: %s", esp_err_to_name(ret));
        goto fail;
    }
    
    /* 初始化驱动 */
    ret = self->driver->init(self->driver_ctx);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Driver init failed: %s", esp_err_to_name(ret));
        goto fail;
    }
    
    /* 创建任务 */
    char task_name[32];
    snprintf(task_name, sizeof(task_name), "sc_%s", self->name);
    
    BaseType_t err = xTaskCreatePinnedToCore(sc_task, task_name,
                                  self->config.task_stack_size,
                                  self, self->config.task_priority,
                                  &self->task, 0);
    if (err != pdPASS) {
        ESP_LOGE(TAG, "Failed to create task");
        ret = ESP_FAIL;
        goto fail;
    }
    
    self->is_initialized = true;
    ESP_LOGI(TAG, "Component '%s' initialized", self->name);
    
    return ESP_OK;

fail:
    if (self->event_group) {
        vEventGroupDelete(self->event_group);
        self->event_group = NULL;
    }
    if (self->mutex) {
        vSemaphoreDelete(self->mutex);
        self->mutex = NULL;
    }
    return ret;
}

esp_err_t sc_component_deinit(sc_component_t *self)
{
    if (self == NULL || !self->is_initialized) {
        return ESP_OK;
    }
    
    /* 停止配网 */
    sc_component_stop(self);
    
    /* 删除任务 */
    if (self->task) {
        vTaskDelete(self->task);
        self->task = NULL;
    }
    
    /* 删除定时器 */
    if (self->timer) {
        xTimerDelete(self->timer, portMAX_DELAY);
        self->timer = NULL;
    }
    
    /* 反初始化各模块 */
    if (self->driver && self->driver->deinit) {
        self->driver->deinit(self->driver_ctx);
    }
    if (self->wifi && self->wifi->deinit) {
        self->wifi->deinit(self->wifi_ctx);
    }
    if (self->storage && self->storage->deinit) {
        self->storage->deinit(self->storage_ctx);
    }
    
    /* 释放资源 */
    if (self->event_group) {
        vEventGroupDelete(self->event_group);
        self->event_group = NULL;
    }
    if (self->mutex) {
        vSemaphoreDelete(self->mutex);
        self->mutex = NULL;
    }
    
    self->is_initialized = false;
    ESP_LOGI(TAG, "Component '%s' deinitialized", self->name);
    
    return ESP_OK;
}

/* ==================== 控制方法 ==================== */

esp_err_t sc_component_start(sc_component_t *self, uint32_t timeout_ms)
{
    if (self == NULL || !self->is_initialized) {
        return ESP_ERR_INVALID_STATE;
    }
    
    sc_lock(self);
    
    if (self->is_running) {
        sc_unlock(self);
        ESP_LOGW(TAG, "Component already running");
        return ESP_ERR_INVALID_STATE;
    }
    
    /* 创建超时定时器 */
    if (timeout_ms > 0) {
        char timer_name[32];
        snprintf(timer_name, sizeof(timer_name), "sc_tmr_%s", self->name);
        
        self->timer = xTimerCreate(timer_name, pdMS_TO_TICKS(timeout_ms),
                                   pdFALSE, self, sc_timer_callback);
        if (self->timer == NULL) {
            sc_unlock(self);
            return ESP_FAIL;
        }
    }
    
    /* 设置启动事件 */
    xEventGroupSetBits(self->event_group, SC_EVENT_START);
    
    self->is_running = true;
    sc_set_status(self, SC_STATUS_STARTING);
    
    sc_unlock(self);
    
    ESP_LOGI(TAG, "SmartConfig started (timeout=%lu ms)", timeout_ms);
    
    return ESP_OK;
}

esp_err_t sc_component_stop(sc_component_t *self)
{
    if (self == NULL || !self->is_initialized) {
        return ESP_OK;
    }
    
    sc_lock(self);
    
    if (!self->is_running) {
        sc_unlock(self);
        return ESP_OK;
    }
    
    /* 设置停止事件 */
    xEventGroupSetBits(self->event_group, SC_EVENT_STOP);
    
    /* 停止定时器 */
    if (self->timer) {
        xTimerStop(self->timer, portMAX_DELAY);
    }
    
    /* 停止驱动 */
    self->driver->stop(self->driver_ctx);
    
    self->is_running = false;
    
    sc_unlock(self);
    
    ESP_LOGI(TAG, "SmartConfig stopped");
    
    return ESP_OK;
}

sc_status_t sc_component_get_status(sc_component_t *self)
{
    if (self == NULL) {
        return SC_STATUS_IDLE;
    }
    
    sc_lock(self);
    sc_status_t status = self->status;
    sc_unlock(self);
    
    return status;
}

/* ==================== 观察者模式 ==================== */

esp_err_t sc_component_attach_observer(sc_component_t *self,
                                        sc_status_callback_t callback,
                                        void *user_data)
{
    if (self == NULL || callback == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    
    sc_lock(self);
    
    /* 查找空位 */
    int slot = -1;
    for (int i = 0; i < SC_MAX_OBSERVERS; i++) {
        if (!self->observers[i].active) {
            slot = i;
            break;
        }
    }
    
    if (slot < 0) {
        sc_unlock(self);
        return ESP_ERR_NO_MEM;
    }
    
    self->observers[slot].callback = callback;
    self->observers[slot].user_data = user_data;
    self->observers[slot].active = true;
    
    sc_unlock(self);
    
    ESP_LOGD(TAG, "Observer attached at slot %d", slot);
    
    return ESP_OK;
}

esp_err_t sc_component_detach_observer(sc_component_t *self,
                                        sc_status_callback_t callback)
{
    if (self == NULL || callback == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    
    sc_lock(self);
    
    for (int i = 0; i < SC_MAX_OBSERVERS; i++) {
        if (self->observers[i].active && 
            self->observers[i].callback == callback) {
            self->observers[i].active = false;
            self->observers[i].callback = NULL;
            self->observers[i].user_data = NULL;
            ESP_LOGD(TAG, "Observer detached from slot %d", i);
            break;
        }
    }
    
    sc_unlock(self);
    
    return ESP_OK;
}

/* ==================== WiFi配置管理 ==================== */

bool sc_component_has_saved_wifi(sc_component_t *self)
{
    if (self == NULL) {
        return false;
    }
    
    sc_lock(self);
    bool has = self->has_saved_wifi;
    sc_unlock(self);
    
    return has;
}

esp_err_t sc_component_get_saved_wifi(sc_component_t *self,
                                       sc_wifi_info_t *info)
{
    if (self == NULL || info == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    
    sc_lock(self);
    
    if (!self->has_saved_wifi) {
        sc_unlock(self);
        return ESP_ERR_NOT_FOUND;
    }
    
    *info = self->wifi_info;
    
    sc_unlock(self);
    
    return ESP_OK;
}

esp_err_t sc_component_clear_wifi(sc_component_t *self)
{
    if (self == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    
    sc_lock(self);
    
    esp_err_t ret = self->storage->clear_wifi_config(self->storage_ctx);
    if (ret == ESP_OK) {
        self->has_saved_wifi = false;
        memset(&self->saved_config, 0, sizeof(self->saved_config));
        memset(&self->wifi_info, 0, sizeof(self->wifi_info));
    }
    
    sc_unlock(self);
    
    ESP_LOGI(TAG, "WiFi config cleared");
    
    return ret;
}

esp_err_t sc_component_connect_saved(sc_component_t *self)
{
    if (self == NULL || !self->is_initialized) {
        return ESP_ERR_INVALID_STATE;
    }
    
    sc_lock(self);
    
    if (!self->has_saved_wifi) {
        sc_unlock(self);
        return ESP_ERR_NOT_FOUND;
    }
    
    ESP_LOGI(TAG, "Connecting to saved WiFi: %s", self->wifi_info.ssid);
    
    esp_err_t ret = self->wifi->connect(self->wifi_ctx, &self->saved_config);
    
    sc_unlock(self);
    
    return ret;
}

sc_config_t sc_component_get_default_config(void)
{
    sc_config_t config = {
        .task_stack_size = 4096,
        .task_priority = 3,
        .auto_save_wifi = true,
        .auto_reconnect = true,
    };
    return config;
}

/* ==================== 私有函数实现 ==================== */

static esp_err_t sc_lock(sc_component_t *self)
{
    if (self && self->mutex) {
        xSemaphoreTake(self->mutex, portMAX_DELAY);
    }
    return ESP_OK;
}

static void sc_unlock(sc_component_t *self)
{
    if (self && self->mutex) {
        xSemaphoreGive(self->mutex);
    }
}

static void sc_set_status(sc_component_t *self, sc_status_t status)
{
    self->status = status;
    sc_notify_observers(self);
}

static void sc_notify_observers(sc_component_t *self)
{
    sc_wifi_info_t info = {0};
    
    /* 复制WiFi信息 */
    if (self->status == SC_STATUS_CONNECTED) {
        info = self->wifi_info;
    }
    
    /* 通知所有观察者 */
    for (int i = 0; i < SC_MAX_OBSERVERS; i++) {
        if (self->observers[i].active && self->observers[i].callback) {
            self->observers[i].callback(self, self->status, 
                                        self->status == SC_STATUS_CONNECTED ? &info : NULL,
                                        self->observers[i].user_data);
        }
    }
}

static void sc_timer_callback(TimerHandle_t xTimer)
{
    sc_component_t *self = (sc_component_t *)pvTimerGetTimerID(xTimer);
    
    ESP_LOGW(TAG, "SmartConfig timeout");
    
    xEventGroupSetBits(self->event_group, SC_EVENT_TIMEOUT);
}

/* ==================== 主任务 ==================== */

static void sc_task(void *pvParameters)
{
    sc_component_t *self = (sc_component_t *)pvParameters;
    EventBits_t events;
    
    ESP_LOGI(TAG, "Task started for component '%s'", self->name);
    
    while (1) {
        /* 等待事件 */
        events = xEventGroupWaitBits(self->event_group,
                                      SC_EVENT_START | SC_EVENT_STOP | 
                                      SC_EVENT_CONNECTED | SC_EVENT_FAILED |
                                      SC_EVENT_TIMEOUT | SC_EVENT_GOT_SSID_PSWD,
                                      pdTRUE, pdFALSE, portMAX_DELAY);
        
        if (events & SC_EVENT_START) {
            /* 启动配网 */
            sc_lock(self);
            
            /* 断开当前连接 */
            self->wifi->disconnect(self->wifi_ctx);
            
            /* 启动驱动 */
            esp_err_t ret = self->driver->start(self->driver_ctx, 0);
            if (ret != ESP_OK) {
                xEventGroupSetBits(self->event_group, SC_EVENT_FAILED);
            } else {
                /* 启动超时定时器 */
                if (self->timer) {
                    xTimerStart(self->timer, portMAX_DELAY);
                }
                sc_set_status(self, SC_STATUS_SCANNING);
            }
            
            sc_unlock(self);
        }
        
        if (events & SC_EVENT_GOT_SSID_PSWD) {
            sc_lock(self);
            
            ESP_LOGI(TAG, "Got SSID and password, connecting...");
            
            /* 停止超时定时器 */
            if (self->timer) {
                xTimerStop(self->timer, portMAX_DELAY);
            }
            
            /* 停止驱动 */
            self->driver->stop(self->driver_ctx);
            
            sc_set_status(self, SC_STATUS_CONNECTING);
            
            /* 连接WiFi */
            esp_err_t ret = self->wifi->connect(self->wifi_ctx, &self->saved_config);
            if (ret != ESP_OK) {
                xEventGroupSetBits(self->event_group, SC_EVENT_FAILED);
            }
            
            sc_unlock(self);
        }
        
        if (events & SC_EVENT_CONNECTED) {
            sc_lock(self);
            
            sc_set_status(self, SC_STATUS_CONNECTED);
            
            /* 自动保存WiFi配置 */
            if (self->config.auto_save_wifi) {
                self->storage->save_wifi_config(self->storage_ctx, &self->saved_config);
                self->has_saved_wifi = true;
            }
            
            self->is_running = false;
            
            sc_unlock(self);
        }
        
        if (events & SC_EVENT_FAILED) {
            sc_lock(self);
            
            sc_set_status(self, SC_STATUS_FAILED);
            
            if (self->timer) {
                xTimerStop(self->timer, portMAX_DELAY);
            }
            
            self->driver->stop(self->driver_ctx);
            self->is_running = false;
            
            sc_unlock(self);
        }
        
        if (events & SC_EVENT_TIMEOUT) {
            xEventGroupSetBits(self->event_group, SC_EVENT_FAILED);
        }
        
        if (events & SC_EVENT_STOP) {
            /* 停止命令已处理 */
        }
    }
}

/* ==================== 供驱动调用的内部API ==================== */

/* 内部函数声明在 smartconfig_component_internal.h 中 */
#include "smartconfig_component_internal.h"

/**
 * @brief 驱动获取SSID/PSWD后调用此函数通知组件
 * 这是驱动和组件之间的内部通信接口
 */
void sc_internal_on_got_ssid_pswd(sc_component_t *self, 
                                   const uint8_t *ssid, 
                                   const uint8_t *password,
                                   const uint8_t *bssid,
                                   uint8_t channel)
{
    if (self == NULL) {
        return;
    }
    
    sc_lock(self);
    
    /* 保存WiFi信息 */
    memset(&self->saved_config, 0, sizeof(self->saved_config));
    memcpy(self->saved_config.sta.ssid, ssid, 32);
    memcpy(self->saved_config.sta.password, password, 64);
    if (bssid) {
        memcpy(self->saved_config.sta.bssid, bssid, 6);
        self->saved_config.sta.bssid_set = true;
    }
    
    /* 保存到info结构 */
    memset(&self->wifi_info, 0, sizeof(self->wifi_info));
    memcpy(self->wifi_info.ssid, ssid, 32);
    self->wifi_info.ssid[32] = '\0';
    memcpy(self->wifi_info.password, password, 64);
    self->wifi_info.password[64] = '\0';
    if (bssid) {
        memcpy(self->wifi_info.bssid, bssid, 6);
    }
    self->wifi_info.channel = channel;
    
    sc_unlock(self);
    
    /* 触发事件 */
    xEventGroupSetBits(self->event_group, SC_EVENT_GOT_SSID_PSWD);
}

/**
 * @brief 驱动状态变化时调用
 */
void sc_internal_on_driver_status(sc_component_t *self, sc_status_t status)
{
    if (self == NULL) {
        return;
    }
    
    sc_lock(self);
    self->status = status;
    sc_unlock(self);
    
    sc_notify_observers(self);
}

/**
 * @brief WiFi连接成功时调用
 */
void sc_internal_on_wifi_connected(sc_component_t *self)
{
    if (self == NULL) {
        return;
    }
    
    xEventGroupSetBits(self->event_group, SC_EVENT_CONNECTED);
}

/**
 * @brief WiFi断开时调用
 */
void sc_internal_on_wifi_disconnected(sc_component_t *self)
{
    if (self == NULL) {
        return;
    }
    
    /* 清理连接状态 */
    sc_lock(self);
    if (self->status == SC_STATUS_CONNECTED) {
        /* 如果配置了自动重连，尝试重新连接 */
        if (self->config.auto_reconnect && self->has_saved_wifi) {
            self->wifi->connect(self->wifi_ctx, &self->saved_config);
        }
    }
    sc_unlock(self);
}
