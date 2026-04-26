/*
 * 应用初始化实现
 */

#include "app_init.h"
#include "app_event.h"
#include "app_wifi.h"
#include "app_led.h"
#include "app_mqtt.h"
#include "app_version.h"
#include "i2c_bus_component.h"
#include "xl9555_component.h"
#include "xl9555_event.h"
#include "xl9555_pins.h"
#include "nvs_flash.h"
#include "esp_log.h"

static const char *TAG = "app_init";

void app_init_task(void *pvParameters) {
    (void)pvParameters;
    
    /* 0. 打印版本信息（最先执行） */
    app_version_log_banner();
    
    ESP_LOGI(TAG, "========== 应用初始化开始 ==========");
    
    /* 1. 基础系统 */
    ESP_LOGI(TAG, "[1/5] 初始化NVS...");
    esp_err_t ret = nvs_flash_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "NVS初始化失败: %s", esp_err_to_name(ret));
    }
    
    /* 2. 事件系统（必须在其他模块之前） */
    ESP_LOGI(TAG, "[2/5] 初始化事件系统...");
    ret = app_event_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "事件系统初始化失败: %s,系统无法正常工作", esp_err_to_name(ret));
        vTaskDelete(NULL);
    }
    
    /* 2.1 发布版本信息事件 */
    const app_version_info_t *ver_info = app_version_get_info();
    app_event_post(APP_EVENT_VERSION_INFO, APP_EVENT_PRIO_LOW, ver_info, sizeof(*ver_info));
    
    /* 3. I2C总线与XL9555扩展芯片 */
    ESP_LOGI(TAG, "[3/5] 初始化XL9555...");
    i2c_bus_dev_t *i2c_bus = i2c_bus_create_esp32(45, 48, 400000);
    if (i2c_bus) {
        ret = i2c_bus_init(i2c_bus);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "I2C总线初始化失败: %s", esp_err_to_name(ret));
        } else {
            xl9555_config_t xl_cfg = {
                .i2c_bus     = i2c_bus,
                .i2c_addr    = 0x20,
                .int_gpio    = 3,
                .default_dir = XL9555_DEFAULT_DIR_MASK,
                .default_out = XL9555_DEFAULT_OUT_STATE,
            };
            xl9555_dev_t *xl9555 = xl9555_create(&xl_cfg);
            if (xl9555) {
                ret = xl9555_init(xl9555);
                if (ret == ESP_OK) {
                    xl9555_key_event_start(xl9555);
                    xl9555_syserr_led_auto_register(xl9555);
                    ESP_LOGI(TAG, "XL9555初始化完成");
                } else {
                    ESP_LOGE(TAG, "XL9555初始化失败: %s", esp_err_to_name(ret));
                }
            } else {
                ESP_LOGE(TAG, "XL9555创建失败");
            }
        }
    } else {
        ESP_LOGE(TAG, "I2C总线创建失败");
    }
    
    /* 4. WiFi（自动连接或进入配网） */
    ESP_LOGI(TAG, "[4/5] 初始化WiFi...");
    ret = app_wifi_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "WiFi初始化失败: %s", esp_err_to_name(ret));
    } else {
        ret = app_wifi_start();
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "WiFi启动失败: %s", esp_err_to_name(ret));
        }
    }
    
    /* 5. MQTT 客户端（初始化但不连接，等 WiFi 就绪后自动连接） */
    ESP_LOGI(TAG, "[5/5] 初始化MQTT...");
    ret = app_mqtt_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "MQTT初始化失败: %s", esp_err_to_name(ret));
    }
    
    /* 6. LED管理器（最后初始化，在所有外设就绪后启动） */
    ESP_LOGI(TAG, "[6/6] 初始化LED...");
    ret = app_led_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "LED初始化失败: %s", esp_err_to_name(ret));
    } else {
        app_led_start();  // 启动LED任务（优先级1，最低）
    }
    
    /* 发布初始化完成事件 */
    app_event_post_simple(APP_EVENT_INIT_DONE);
    
    ESP_LOGI(TAG, "========== 应用初始化完成 ==========");
    ESP_LOGI(TAG, "WiFi状态: %s", 
             app_wifi_is_connected() ? "已连接" : 
             (app_wifi_get_state() == APP_WIFI_STATE_PROVISIONING ? "配网中" : "等待配网"));
    
    vTaskDelete(NULL);
}
