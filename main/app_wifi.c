/*
 * WiFi管理器实现
 */

#include "app_wifi.h"
#include "smartconfig_component.h"
#include "app_event.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include "freertos/queue.h"
#include "esp_log.h"

#define TAG "app_wifi"
#define PROV_TIMEOUT_MS 120000   /* 2分钟配网超时 */
#define PROV_RETRY_DELAY_MS 5000 /* 重试前延时 */

static sc_component_t *s_sc = NULL;
static app_wifi_state_t s_state = APP_WIFI_STATE_IDLE;
static TimerHandle_t s_prov_timer = NULL;
static bool s_prov_timeout_flag = false; /* 配网超时标志 */

static QueueHandle_t s_wifi_cmd_queue = NULL;

typedef enum {
    WIFI_CMD_START_PROVISIONING,
    WIFI_CMD_STOP,
    WIFI_CMD_CLEAR_CONFIG,
} wifi_cmd_t;

static esp_err_t app_wifi_start_provisioning_internal(void);
static void app_wifi_ctrl_task(void *pvParam);

/* ==================== 内部函数 ==================== */

/**
 * @brief 发布WiFi状态事件
 */
static void post_wifi_event(app_event_id_t event_id, const char *ssid)
{
    if (ssid)
    {
        struct
        {
            char ssid[33];
        } data = {0};
        strncpy(data.ssid, ssid, 32);
        app_event_post(event_id, APP_EVENT_PRIO_HIGH, &data, sizeof(data));
    }
    else
    {
        app_event_post_simple(event_id);
    }
}

/**
 * @brief SmartConfig组件回调
 */
static void sc_status_callback(sc_component_t *component,
                               sc_status_t status,
                               const sc_wifi_info_t *info,
                               void *user_data)
{
    (void)component;
    (void)user_data;

    switch (status)
    {
    case SC_STATUS_STARTING:
        s_state = APP_WIFI_STATE_PROVISIONING;
        post_wifi_event(APP_EVENT_PROV_START, NULL);
        break;

    case SC_STATUS_SCANNING:
        s_state = APP_WIFI_STATE_PROVISIONING;
        post_wifi_event(APP_EVENT_WIFI_SCANNING, NULL);
        break;

    case SC_STATUS_CONNECTING:
        s_state = APP_WIFI_STATE_CONNECTING;
        post_wifi_event(APP_EVENT_WIFI_CONNECTING, NULL);
        break;

    case SC_STATUS_CONNECTED:
        s_state = APP_WIFI_STATE_CONNECTED;
        s_prov_timeout_flag = false;
        post_wifi_event(APP_EVENT_WIFI_CONNECTED, info ? info->ssid : NULL);
        break;

    case SC_STATUS_FAILED:
        s_state = APP_WIFI_STATE_ERROR;
        post_wifi_event(APP_EVENT_WIFI_FAILED, NULL);
        break;

    case SC_STATUS_STOPPED:
        if (s_state == APP_WIFI_STATE_PROVISIONING)
        {
            post_wifi_event(APP_EVENT_PROV_STOPPED, NULL);
        }
        s_state = APP_WIFI_STATE_IDLE;
        break;

    default:
        break;
    }
}

/**
 * @brief 配网超时定时器回调
 */
static void prov_timeout_callback(TimerHandle_t xTimer)
{
    (void)xTimer;

    ESP_LOGW(TAG, "配网超时（2分钟），停止扫描");

    if (s_sc)
    {
        sc_component_stop(s_sc);
    }

    s_prov_timeout_flag = true;
    s_state = APP_WIFI_STATE_PROV_TIMEOUT;

    post_wifi_event(APP_EVENT_PROV_TIMEOUT, NULL);
}

/**
 * @brief 尝试连接保存的WiFi
 */
static esp_err_t try_connect_saved(void)
{
    if (!sc_component_has_saved_wifi(s_sc))
    {
        return ESP_ERR_NOT_FOUND;
    }

    sc_wifi_info_t info;
    if (sc_component_get_saved_wifi(s_sc, &info) != ESP_OK)
    {
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "尝试连接保存的WiFi: %s", info.ssid);
    s_state = APP_WIFI_STATE_CONNECTING;

    return sc_component_connect_saved(s_sc);
}

/**
 * @brief 启动配网（带超时）
 */
static esp_err_t start_provisioning_with_timeout(void)
{
    if (!s_sc)
        return ESP_ERR_INVALID_STATE;

    ESP_LOGI(TAG, "启动配网扫描，超时时间: %d秒", PROV_TIMEOUT_MS / 1000);

    s_prov_timeout_flag = false;

    /* 创建超时定时器（仅第一次） */
    if (s_prov_timer == NULL)
    {
        s_prov_timer = xTimerCreate("prov_timer",
                                    pdMS_TO_TICKS(PROV_TIMEOUT_MS),
                                    pdFALSE, NULL,
                                    prov_timeout_callback);
        if (!s_prov_timer)
        {
            return ESP_FAIL;
        }
    }

    /* 启动定时器 */
    xTimerStart(s_prov_timer, 0);

    /* 启动配网（无组件内部超时，由本层管理） */
    return sc_component_start(s_sc, 0);
}

/* ==================== 接口实现 ==================== */

esp_err_t app_wifi_init(void)
{
    ESP_LOGI(TAG, "初始化WiFi管理器");

    /* 创建SmartConfig组件 */
    extern void *sc_driver_esptouch_create_ctx(void);
    extern void *sc_storage_nvs_create_ctx(void);
    extern void *sc_wifi_esp32_create_ctx(void);
    extern void sc_driver_esptouch_set_component(void *ctx, sc_component_t *comp);
    extern void sc_wifi_esp32_set_component(void *ctx, sc_component_t *comp);

    void *driver_ctx = sc_driver_esptouch_create_ctx();
    void *storage_ctx = sc_storage_nvs_create_ctx();
    void *wifi_ctx = sc_wifi_esp32_create_ctx();

    if (!driver_ctx || !storage_ctx || !wifi_ctx)
    {
        return ESP_FAIL;
    }

    sc_config_t config = sc_component_get_default_config();
    s_sc = sc_component_create("app_wifi",
                               &sc_driver_esptouch, driver_ctx,
                               &sc_storage_nvs, storage_ctx,
                               &sc_wifi_esp32, wifi_ctx,
                               &config);

    if (!s_sc)
        return ESP_FAIL;

    sc_driver_esptouch_set_component(driver_ctx, s_sc);
    sc_wifi_esp32_set_component(wifi_ctx, s_sc);

    ESP_ERROR_CHECK(sc_component_init(s_sc));
    ESP_ERROR_CHECK(sc_component_attach_observer(s_sc, sc_status_callback, NULL));

    s_state = APP_WIFI_STATE_IDLE;

    ESP_LOGI(TAG, "WiFi管理器初始化完成");

    s_wifi_cmd_queue = xQueueCreate(4, sizeof(wifi_cmd_t));
    if (!s_wifi_cmd_queue) {
        ESP_LOGE(TAG, "Failed to create WiFi cmd queue");
        return ESP_ERR_NO_MEM;
    }

    xTaskCreatePinnedToCore(app_wifi_ctrl_task, "wifi_ctrl", 2048, NULL, 4, NULL, 0);

    return ESP_OK;
}

esp_err_t app_wifi_start(void)
{
    if (!s_sc)
        return ESP_ERR_INVALID_STATE;

    /* 尝试连接保存的WiFi */
    if (try_connect_saved() == ESP_OK)
    {
        return ESP_OK;
    }

    /* 无保存WiFi，启动配网 */
    ESP_LOGI(TAG, "无保存WiFi配置，进入配网模式");
    return start_provisioning_with_timeout();
}

esp_err_t app_wifi_stop(void)
{
    if (s_prov_timer)
    {
        xTimerStop(s_prov_timer, 0);
    }
    return s_sc ? sc_component_stop(s_sc) : ESP_OK;
}

app_wifi_state_t app_wifi_get_state(void)
{
    return s_state;
}

bool app_wifi_is_connected(void)
{
    return s_state == APP_WIFI_STATE_CONNECTED;
}

esp_err_t app_wifi_get_ssid(char *ssid, size_t len)
{
    if (!s_sc || !ssid || len < 1)
        return ESP_ERR_INVALID_ARG;

    sc_wifi_info_t info;
    if (sc_component_get_saved_wifi(s_sc, &info) == ESP_OK)
    {
        strncpy(ssid, info.ssid, len - 1);
        ssid[len - 1] = '\0';
        return ESP_OK;
    }

    return ESP_ERR_NOT_FOUND;
}

static esp_err_t app_wifi_start_provisioning_internal(void)
{
    if (!s_sc)
        return ESP_ERR_INVALID_STATE;

    /* 检查状态 */
    if (s_state == APP_WIFI_STATE_CONNECTED)
    {
        ESP_LOGW(TAG, "WiFi已连接，无需配网");
        return ESP_ERR_INVALID_STATE;
    }

    if (s_state == APP_WIFI_STATE_PROVISIONING)
    {
        ESP_LOGW(TAG, "配网已在进行中");
        return ESP_ERR_INVALID_STATE;
    }

    /* 只有在超时或空闲状态才允许重新启动 */
    if (s_state != APP_WIFI_STATE_PROV_TIMEOUT &&
        s_state != APP_WIFI_STATE_IDLE &&
        s_state != APP_WIFI_STATE_ERROR)
    {
        return ESP_ERR_INVALID_STATE;
    }

    ESP_LOGI(TAG, "手动重启配网");

    /* 延时后启动，给用户反应时间 */
    vTaskDelay(pdMS_TO_TICKS(PROV_RETRY_DELAY_MS));

    return start_provisioning_with_timeout();
}

static void app_wifi_ctrl_task(void *pvParam)
{
    (void)pvParam;
    wifi_cmd_t cmd;

    for (;;) {
        if (xQueueReceive(s_wifi_cmd_queue, &cmd, portMAX_DELAY)) {
            switch (cmd) {
                case WIFI_CMD_START_PROVISIONING:
                    app_wifi_start_provisioning_internal();
                    break;
                case WIFI_CMD_STOP:
                    app_wifi_stop();
                    break;
                case WIFI_CMD_CLEAR_CONFIG:
                    app_wifi_clear_config();
                    break;
                default:
                    break;
            }
        }
    }
}

esp_err_t app_wifi_start_provisioning(void)
{
    if (xPortGetCoreID() == 0) {
        return app_wifi_start_provisioning_internal();
    }

    if (!s_wifi_cmd_queue)
        return ESP_ERR_INVALID_STATE;

    wifi_cmd_t cmd = WIFI_CMD_START_PROVISIONING;
    BaseType_t ret = xQueueSend(s_wifi_cmd_queue, &cmd, pdMS_TO_TICKS(100));

    return (ret == pdTRUE) ? ESP_OK : ESP_FAIL;
}

esp_err_t app_wifi_clear_config(void)
{
    if (!s_sc)
        return ESP_ERR_INVALID_STATE;
    return sc_component_clear_wifi(s_sc);
}
