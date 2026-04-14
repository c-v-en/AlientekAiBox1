/*
 * ESPTOUCH 驱动实现
 * 实现 sc_driver_iface_t 接口
 */

#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_smartconfig.h"
#include "esp_log.h"
#include "smartconfig_component.h"
#include "smartconfig_component_internal.h"

static const char *TAG = "sc_esptouch";

/* 驱动上下文结构 */
typedef struct {
    sc_component_t *component;
    sc_status_t status;
    bool is_started;
} esptouch_ctx_t;

/* 内部函数声明 */
static void esptouch_event_handler(void *arg, esp_event_base_t event_base,
                                    int32_t event_id, void *event_data);
static esp_err_t register_event_handlers(void);
static esp_err_t unregister_event_handlers(void);

/* 全局实例（用于事件处理） */
static esptouch_ctx_t *s_ctx = NULL;
static bool s_events_registered = false;

/* ==================== 驱动接口实现 ==================== */

static esp_err_t esptouch_init(void *ctx)
{
    ESP_LOGI(TAG, "ESPTOUCH driver init");
    
    esptouch_ctx_t *et_ctx = (esptouch_ctx_t *)ctx;
    if (et_ctx == NULL) {
        /* 创建默认上下文 */
        et_ctx = calloc(1, sizeof(esptouch_ctx_t));
        if (et_ctx == NULL) {
            return ESP_ERR_NO_MEM;
        }
    }
    
    et_ctx->status = SC_STATUS_IDLE;
    et_ctx->is_started = false;
    
    s_ctx = et_ctx;
    
    /* 注册事件处理器 */
    ESP_ERROR_CHECK(register_event_handlers());
    
    return ESP_OK;
}

static esp_err_t esptouch_deinit(void *ctx)
{
    ESP_LOGI(TAG, "ESPTOUCH driver deinit");
    
    esptouch_ctx_t *et_ctx = (esptouch_ctx_t *)ctx;
    
    /* 确保已停止 */
    if (et_ctx && et_ctx->is_started) {
        esp_smartconfig_stop();
        et_ctx->is_started = false;
    }
    
    /* 注销事件处理器 */
    unregister_event_handlers();
    
    s_ctx = NULL;
    
    if (ctx) {
        free(ctx);
    }
    
    return ESP_OK;
}

static esp_err_t esptouch_start(void *ctx, uint32_t timeout_ms)
{
    ESP_LOGI(TAG, "ESPTOUCH driver start");
    
    esptouch_ctx_t *et_ctx = (esptouch_ctx_t *)ctx;
    
    if (et_ctx == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    
    /* 设置配网类型 */
    ESP_ERROR_CHECK(esp_smartconfig_set_type(SC_TYPE_ESPTOUCH));
    
    /* 启动 SmartConfig */
    smartconfig_start_config_t cfg = SMARTCONFIG_START_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_smartconfig_start(&cfg));
    
    et_ctx->is_started = true;
    et_ctx->status = SC_STATUS_SCANNING;
    
    ESP_LOGI(TAG, "SmartConfig started");
    
    return ESP_OK;
}

static esp_err_t esptouch_stop(void *ctx)
{
    ESP_LOGI(TAG, "ESPTOUCH driver stop");
    
    esptouch_ctx_t *et_ctx = (esptouch_ctx_t *)ctx;
    
    if (et_ctx && et_ctx->is_started) {
        esp_smartconfig_stop();
        et_ctx->is_started = false;
    }
    
    et_ctx->status = SC_STATUS_STOPPED;
    
    return ESP_OK;
}

static sc_status_t esptouch_get_status(void *ctx)
{
    esptouch_ctx_t *et_ctx = (esptouch_ctx_t *)ctx;
    
    if (et_ctx == NULL) {
        return SC_STATUS_IDLE;
    }
    
    return et_ctx->status;
}

/* ==================== 驱动接口结构体 ==================== */

const sc_driver_iface_t sc_driver_esptouch = {
    .name = "esptouch",
    .init = esptouch_init,
    .deinit = esptouch_deinit,
    .start = esptouch_start,
    .stop = esptouch_stop,
    .get_status = esptouch_get_status,
};

/* ==================== 事件处理 ==================== */

static esp_err_t register_event_handlers(void)
{
    if (s_events_registered) {
        return ESP_OK;
    }
    
    ESP_ERROR_CHECK(esp_event_handler_register(SC_EVENT, ESP_EVENT_ANY_ID,
                                                &esptouch_event_handler, NULL));
    
    s_events_registered = true;
    
    return ESP_OK;
}

static esp_err_t unregister_event_handlers(void)
{
    if (!s_events_registered) {
        return ESP_OK;
    }
    
    esp_event_handler_unregister(SC_EVENT, ESP_EVENT_ANY_ID,
                                  &esptouch_event_handler);
    
    s_events_registered = false;
    
    return ESP_OK;
}

static void esptouch_event_handler(void *arg, esp_event_base_t event_base,
                                    int32_t event_id, void *event_data)
{
    if (s_ctx == NULL || s_ctx->component == NULL) {
        return;
    }
    
    sc_component_t *component = s_ctx->component;
    
    if (event_base == SC_EVENT) {
        switch (event_id) {
            case SC_EVENT_SCAN_DONE:
                ESP_LOGI(TAG, "Scan done");
                s_ctx->status = SC_STATUS_SCANNING;
                sc_internal_on_driver_status(component, SC_STATUS_SCANNING);
                break;
                
            case SC_EVENT_FOUND_CHANNEL:
                ESP_LOGI(TAG, "Found channel");
                s_ctx->status = SC_STATUS_FOUND_CHANNEL;
                sc_internal_on_driver_status(component, SC_STATUS_FOUND_CHANNEL);
                break;
                
            case SC_EVENT_GOT_SSID_PSWD: {
                ESP_LOGI(TAG, "Got SSID and password");
                s_ctx->status = SC_STATUS_GETTING_SSID_PSWD;
                sc_internal_on_driver_status(component, SC_STATUS_GETTING_SSID_PSWD);
                
                smartconfig_event_got_ssid_pswd_t *evt = 
                    (smartconfig_event_got_ssid_pswd_t *)event_data;
                
                uint8_t ssid[33] = {0};
                uint8_t password[65] = {0};
                uint8_t bssid[6] = {0};
                
                memcpy(ssid, evt->ssid, sizeof(evt->ssid));
                memcpy(password, evt->password, sizeof(evt->password));
                memcpy(bssid, evt->bssid, sizeof(evt->bssid));
                
                ESP_LOGI(TAG, "SSID: %s", ssid);
                ESP_LOGI(TAG, "Password: %s", password);
                
                /* 通知组件 */
                sc_internal_on_got_ssid_pswd(component, ssid, password,
                                              evt->bssid_set ? bssid : NULL,
                                              0);
                break;
            }
            
            case SC_EVENT_SEND_ACK_DONE:
                ESP_LOGI(TAG, "Send ACK done");
                break;
                
            default:
                break;
        }
    }
}

/* ==================== 驱动上下文创建 ==================== */

/**
 * @brief 创建 ESPTOUCH 驱动上下文
 * 在创建组件前调用，获取上下文指针传递给 sc_component_create
 */
esptouch_ctx_t* sc_driver_esptouch_create_ctx(void)
{
    esptouch_ctx_t *ctx = calloc(1, sizeof(esptouch_ctx_t));
    if (ctx) {
        /* 保存上下文指针，供事件处理使用 */
        if (s_ctx == NULL) {
            s_ctx = ctx;
        }
    }
    return ctx;
}

/**
 * @brief 设置组件引用
 * 在组件创建后调用，建立驱动和组件的关联
 */
void sc_driver_esptouch_set_component(esptouch_ctx_t *ctx, sc_component_t *component)
{
    if (ctx) {
        ctx->component = component;
    }
}
