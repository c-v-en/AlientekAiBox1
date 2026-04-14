/*
 * NVS 存储实现
 * 实现 sc_storage_iface_t 接口
 */

#include <string.h>
#include "nvs_flash.h"
#include "nvs.h"
#include "esp_log.h"
#include "smartconfig_component.h"

static const char *TAG = "sc_nvs";

#define SC_NVS_NAMESPACE    "smartconfig"
#define SC_NVS_KEY_WIFI     "wifi_config"

/* 存储上下文 */
typedef struct {
    nvs_handle_t handle;
    bool initialized;
} nvs_storage_ctx_t;

/* ==================== 接口实现 ==================== */

static esp_err_t nvs_storage_init(void *ctx)
{
    ESP_LOGI(TAG, "NVS storage init");
    
    nvs_storage_ctx_t *nvs_ctx = (nvs_storage_ctx_t *)ctx;
    if (nvs_ctx == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    
    /* 打开命名空间 */
    esp_err_t ret = nvs_open(SC_NVS_NAMESPACE, NVS_READWRITE, &nvs_ctx->handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to open NVS namespace: %s", esp_err_to_name(ret));
        return ret;
    }
    
    nvs_ctx->initialized = true;
    
    ESP_LOGI(TAG, "NVS storage initialized");
    
    return ESP_OK;
}

static esp_err_t nvs_storage_load_wifi_config(void *ctx, wifi_config_t *config)
{
    nvs_storage_ctx_t *nvs_ctx = (nvs_storage_ctx_t *)ctx;
    
    if (nvs_ctx == NULL || config == NULL || !nvs_ctx->initialized) {
        return ESP_ERR_INVALID_ARG;
    }
    
    size_t length = sizeof(wifi_config_t);
    esp_err_t ret = nvs_get_blob(nvs_ctx->handle, SC_NVS_KEY_WIFI, config, &length);
    
    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "WiFi config loaded from NVS");
    } else if (ret == ESP_ERR_NVS_NOT_FOUND) {
        ESP_LOGW(TAG, "WiFi config not found in NVS");
    } else {
        ESP_LOGE(TAG, "Failed to load WiFi config: %s", esp_err_to_name(ret));
    }
    
    return ret;
}

static esp_err_t nvs_storage_save_wifi_config(void *ctx, const wifi_config_t *config)
{
    nvs_storage_ctx_t *nvs_ctx = (nvs_storage_ctx_t *)ctx;
    
    if (nvs_ctx == NULL || config == NULL || !nvs_ctx->initialized) {
        return ESP_ERR_INVALID_ARG;
    }
    
    esp_err_t ret = nvs_set_blob(nvs_ctx->handle, SC_NVS_KEY_WIFI, 
                                  config, sizeof(wifi_config_t));
    if (ret == ESP_OK) {
        ret = nvs_commit(nvs_ctx->handle);
        if (ret == ESP_OK) {
            ESP_LOGI(TAG, "WiFi config saved to NVS");
        }
    }
    
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to save WiFi config: %s", esp_err_to_name(ret));
    }
    
    return ret;
}

static esp_err_t nvs_storage_clear_wifi_config(void *ctx)
{
    nvs_storage_ctx_t *nvs_ctx = (nvs_storage_ctx_t *)ctx;
    
    if (nvs_ctx == NULL || !nvs_ctx->initialized) {
        return ESP_ERR_INVALID_ARG;
    }
    
    esp_err_t ret = nvs_erase_key(nvs_ctx->handle, SC_NVS_KEY_WIFI);
    if (ret == ESP_OK || ret == ESP_ERR_NVS_NOT_FOUND) {
        nvs_commit(nvs_ctx->handle);
        ESP_LOGI(TAG, "WiFi config cleared from NVS");
        return ESP_OK;
    }
    
    ESP_LOGE(TAG, "Failed to clear WiFi config: %s", esp_err_to_name(ret));
    return ret;
}

static bool nvs_storage_has_config(void *ctx)
{
    nvs_storage_ctx_t *nvs_ctx = (nvs_storage_ctx_t *)ctx;
    
    if (nvs_ctx == NULL || !nvs_ctx->initialized) {
        return false;
    }
    
    wifi_config_t config;
    size_t length = sizeof(config);
    esp_err_t ret = nvs_get_blob(nvs_ctx->handle, SC_NVS_KEY_WIFI, &config, &length);
    
    return (ret == ESP_OK);
}

/* ==================== 接口结构体 ==================== */

const sc_storage_iface_t sc_storage_nvs = {
    .init = nvs_storage_init,
    .load_wifi_config = nvs_storage_load_wifi_config,
    .save_wifi_config = nvs_storage_save_wifi_config,
    .clear_wifi_config = nvs_storage_clear_wifi_config,
    .has_config = nvs_storage_has_config,
};

/* ==================== 上下文创建 ==================== */

/**
 * @brief 创建 NVS 存储上下文
 */
nvs_storage_ctx_t* sc_storage_nvs_create_ctx(void)
{
    nvs_storage_ctx_t *ctx = calloc(1, sizeof(nvs_storage_ctx_t));
    if (ctx == NULL) {
        ESP_LOGE(TAG, "Failed to allocate NVS context");
    }
    return ctx;
}
