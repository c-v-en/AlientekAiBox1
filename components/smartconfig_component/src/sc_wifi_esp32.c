/*
 * ESP32 WiFi 管理实现
 * 实现 sc_wifi_iface_t 接口
 */

#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "smartconfig_component.h"
#include "smartconfig_component_internal.h"

static const char *TAG = "sc_wifi_esp32";

/* WiFi上下文 */
typedef struct {
    sc_component_t *component;
    bool initialized;
    bool connected;
} esp32_wifi_ctx_t;

/* 全局实例 */
static esp32_wifi_ctx_t *s_wifi_ctx = NULL;
static bool s_events_registered = false;

/* 内部函数声明 */
static void wifi_event_handler(void *arg, esp_event_base_t event_base,
                                int32_t event_id, void *event_data);
static esp_err_t register_wifi_handlers(esp32_wifi_ctx_t *ctx);
static esp_err_t unregister_wifi_handlers(void);

/* ==================== 接口实现 ==================== */

static esp_err_t esp32_wifi_init(void *ctx)
{
    ESP_LOGI(TAG, "ESP32 WiFi init");
    
    esp32_wifi_ctx_t *wifi_ctx = (esp32_wifi_ctx_t *)ctx;
    
    /* 初始化网络接口 */
    ESP_ERROR_CHECK(esp_netif_init());
    
    /* 创建默认事件循环 */
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    
    /* 创建默认WiFi STA */
    esp_netif_create_default_wifi_sta();
    
    /* 初始化WiFi */
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    
    /* 设置模式 */
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    
    /* 启动WiFi */
    ESP_ERROR_CHECK(esp_wifi_start());
    
    /* 注册事件处理器 */
    if (wifi_ctx) {
        register_wifi_handlers(wifi_ctx);
        wifi_ctx->initialized = true;
        s_wifi_ctx = wifi_ctx;
    }
    
    ESP_LOGI(TAG, "ESP32 WiFi initialized");
    
    return ESP_OK;
}

static esp_err_t esp32_wifi_deinit(void *ctx)
{
    ESP_LOGI(TAG, "ESP32 WiFi deinit");
    
    esp32_wifi_ctx_t *wifi_ctx = (esp32_wifi_ctx_t *)ctx;
    
    /* 断开连接 */
    esp_wifi_disconnect();
    
    /* 停止WiFi */
    esp_wifi_stop();
    
    /* 反初始化 */
    esp_wifi_deinit();
    
    /* 注销事件 */
    unregister_wifi_handlers();
    
    if (wifi_ctx) {
        wifi_ctx->initialized = false;
        wifi_ctx->connected = false;
    }
    
    s_wifi_ctx = NULL;
    
    return ESP_OK;
}

static esp_err_t esp32_wifi_connect(void *ctx, const wifi_config_t *config)
{
    ESP_LOGI(TAG, "ESP32 WiFi connect");
    
    esp32_wifi_ctx_t *wifi_ctx = (esp32_wifi_ctx_t *)ctx;
    (void)wifi_ctx;  /* 暂时未使用，保留接口一致性 */
    
    if (config == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    
    /* 断开当前连接 */
    esp_wifi_disconnect();
    
    /* 设置配置 */
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, (wifi_config_t *)config));
    
    /* 开始连接 */
    ESP_ERROR_CHECK(esp_wifi_connect());
    
    ESP_LOGI(TAG, "Connecting to %s", config->sta.ssid);
    
    return ESP_OK;
}

static esp_err_t esp32_wifi_disconnect(void *ctx)
{
    ESP_LOGI(TAG, "ESP32 WiFi disconnect");
    
    esp_err_t ret = esp_wifi_disconnect();
    
    esp32_wifi_ctx_t *wifi_ctx = (esp32_wifi_ctx_t *)ctx;
    if (wifi_ctx) {
        wifi_ctx->connected = false;
    }
    
    return ret;
}

static bool esp32_wifi_is_connected(void *ctx)
{
    esp32_wifi_ctx_t *wifi_ctx = (esp32_wifi_ctx_t *)ctx;
    
    if (wifi_ctx == NULL) {
        return false;
    }
    
    return wifi_ctx->connected;
}

/* ==================== 接口结构体 ==================== */

const sc_wifi_iface_t sc_wifi_esp32 = {
    .init = esp32_wifi_init,
    .deinit = esp32_wifi_deinit,
    .connect = esp32_wifi_connect,
    .disconnect = esp32_wifi_disconnect,
    .is_connected = esp32_wifi_is_connected,
};

/* ==================== 事件处理 ==================== */

static esp_err_t register_wifi_handlers(esp32_wifi_ctx_t *ctx)
{
    if (s_events_registered) {
        return ESP_OK;
    }
    
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID,
                                                &wifi_event_handler, ctx));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP,
                                                &wifi_event_handler, ctx));
    
    s_events_registered = true;
    
    return ESP_OK;
}

static esp_err_t unregister_wifi_handlers(void)
{
    if (!s_events_registered) {
        return ESP_OK;
    }
    
    esp_event_handler_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID,
                                  &wifi_event_handler);
    esp_event_handler_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP,
                                  &wifi_event_handler);
    
    s_events_registered = false;
    
    return ESP_OK;
}

static void wifi_event_handler(void *arg, esp_event_base_t event_base,
                                int32_t event_id, void *event_data)
{
    esp32_wifi_ctx_t *wifi_ctx = (esp32_wifi_ctx_t *)arg;
    
    if (wifi_ctx == NULL) {
        return;
    }
    
    if (event_base == WIFI_EVENT) {
        switch (event_id) {
            case WIFI_EVENT_STA_START:
                ESP_LOGI(TAG, "WiFi STA started");
                break;
                
            case WIFI_EVENT_STA_DISCONNECTED: {
                wifi_event_sta_disconnected_t *event = 
                    (wifi_event_sta_disconnected_t *)event_data;
                ESP_LOGW(TAG, "WiFi disconnected, reason: %d", event->reason);
                
                wifi_ctx->connected = false;
                
                /* 通知组件 */
                if (wifi_ctx->component) {
                    sc_internal_on_wifi_disconnected(wifi_ctx->component);
                }
                break;
            }
            
            case WIFI_EVENT_STA_CONNECTED:
                ESP_LOGI(TAG, "WiFi connected");
                break;
                
            default:
                break;
        }
    } else if (event_base == IP_EVENT) {
        switch (event_id) {
            case IP_EVENT_STA_GOT_IP: {
                ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
                ESP_LOGI(TAG, "Got IP: " IPSTR, IP2STR(&event->ip_info.ip));
                
                wifi_ctx->connected = true;
                
                /* 通知组件 */
                if (wifi_ctx->component) {
                    sc_internal_on_wifi_connected(wifi_ctx->component);
                }
                break;
            }
            
            default:
                break;
        }
    }
}

/* ==================== 上下文创建 ==================== */

/**
 * @brief 创建 ESP32 WiFi 上下文
 */
esp32_wifi_ctx_t* sc_wifi_esp32_create_ctx(void)
{
    esp32_wifi_ctx_t *ctx = calloc(1, sizeof(esp32_wifi_ctx_t));
    if (ctx == NULL) {
        ESP_LOGE(TAG, "Failed to allocate WiFi context");
    }
    return ctx;
}

/**
 * @brief 设置组件引用
 */
void sc_wifi_esp32_set_component(esp32_wifi_ctx_t *ctx, sc_component_t *component)
{
    if (ctx) {
        ctx->component = component;
    }
}
