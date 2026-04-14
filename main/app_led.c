/*
 * Application LED Manager Implementation
 * GPIO4 LED控制（系统状态指示）
 */

#include "app_led.h"
#include "led_gpio.h"
#include "led_driver.h"
#include "app_event.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#define TAG "app_led"

#define LED_GPIO_NUM            4
#define LED_GPIO_ACTIVE_LOW     true

#define LED_BLINK_PERIOD_IDLE_MS        500     // 1Hz闪烁周期
#define LED_BLINK_PERIOD_PROV_MS        100     // 5Hz闪烁周期

static struct {
    led_device_t *gpio_led;
    led_system_state_t state;
    TaskHandle_t task;
    bool running;
} s_led_mgr = {0};

/* ==================== 内部函数 ==================== */

static void led_task(void *pvParam) {
    (void)pvParam;
    
    uint32_t on_ms = LED_BLINK_PERIOD_IDLE_MS;
    uint32_t off_ms = LED_BLINK_PERIOD_IDLE_MS;
    led_system_state_t last_state = LED_SYS_IDLE;
    
    while (s_led_mgr.running) {
        // 状态变化时更新参数
        if (s_led_mgr.state != last_state) {
            last_state = s_led_mgr.state;
            
            switch (s_led_mgr.state) {
                case LED_SYS_IDLE:
                    on_ms = LED_BLINK_PERIOD_IDLE_MS;
                    off_ms = LED_BLINK_PERIOD_IDLE_MS;
                    ESP_LOGI(TAG, "State: IDLE (GPIO4 1Hz)");
                    break;
                    
                case LED_SYS_PROVISIONING:
                    on_ms = LED_BLINK_PERIOD_PROV_MS;
                    off_ms = LED_BLINK_PERIOD_PROV_MS;
                    ESP_LOGI(TAG, "State: PROVISIONING (GPIO4 5Hz)");
                    break;
                    
                case LED_SYS_PROV_TIMEOUT:
                    on_ms = LED_BLINK_PERIOD_IDLE_MS;
                    off_ms = LED_BLINK_PERIOD_IDLE_MS;
                    ESP_LOGI(TAG, "State: PROV_TIMEOUT (GPIO4 1Hz)");
                    break;
            }
        }
        
        // 执行GPIO4闪烁
        led_on(s_led_mgr.gpio_led);
        vTaskDelay(pdMS_TO_TICKS(on_ms));
        
        if (off_ms > 0 && s_led_mgr.running) {
            led_off(s_led_mgr.gpio_led);
            vTaskDelay(pdMS_TO_TICKS(off_ms));
        }
    }
    
    vTaskDelete(NULL);
}

/* 事件处理回调 */
static void led_event_handler(const app_event_t *event, void *user_data) {
    (void)user_data;
    
    switch (event->id) {
        case APP_EVENT_PROV_START:
            app_led_set_state(LED_SYS_PROVISIONING);
            break;
            
        case APP_EVENT_WIFI_CONNECTED:
            app_led_set_state(LED_SYS_IDLE);
            break;
            
        case APP_EVENT_PROV_TIMEOUT:
            app_led_set_state(LED_SYS_PROV_TIMEOUT);
            break;
            
        default:
            break;
    }
}

/* ==================== 接口实现 ==================== */

esp_err_t app_led_init(void) {
    ESP_LOGI(TAG, "初始化LED管理器...");
    
    /* 创建GPIO4 LED */
    led_gpio_ctx_t *gpio_ctx = led_gpio_create_ctx(LED_GPIO_NUM, LED_GPIO_ACTIVE_LOW);
    if (!gpio_ctx) {
        ESP_LOGE(TAG, "Failed to create GPIO LED context");
        return ESP_ERR_NO_MEM;
    }
    
    s_led_mgr.gpio_led = led_device_create(&led_gpio_driver, gpio_ctx);
    if (!s_led_mgr.gpio_led) {
        free(gpio_ctx);
        return ESP_ERR_NO_MEM;
    }
    
    esp_err_t ret = led_init(s_led_mgr.gpio_led);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to init GPIO LED: %s", esp_err_to_name(ret));
        return ret;
    }
    
    /* 注册事件监听 */
    ret = app_event_register_listener(APP_EVENT_PROV_START, led_event_handler, NULL);
    if (ret != ESP_OK) ESP_LOGW(TAG, "Failed to register prov_start listener");
    
    ret = app_event_register_listener(APP_EVENT_WIFI_CONNECTED, led_event_handler, NULL);
    if (ret != ESP_OK) ESP_LOGW(TAG, "Failed to register wifi_connected listener");
    
    ret = app_event_register_listener(APP_EVENT_PROV_TIMEOUT, led_event_handler, NULL);
    if (ret != ESP_OK) ESP_LOGW(TAG, "Failed to register prov_timeout listener");
    
    s_led_mgr.state = LED_SYS_IDLE;
    s_led_mgr.running = false;
    
    ESP_LOGI(TAG, "LED manager initialized");
    return ESP_OK;
}

esp_err_t app_led_start(void) {
    if (s_led_mgr.running) return ESP_OK;
    
    ESP_LOGI(TAG, "启动LED任务（优先级1）");
    
    s_led_mgr.running = true;
    
    BaseType_t ret = xTaskCreate(led_task, "led_task", 2048, NULL, 1, &s_led_mgr.task);
    if (ret != pdPASS) {
        s_led_mgr.running = false;
        return ESP_FAIL;
    }
    
    return ESP_OK;
}

void app_led_set_state(led_system_state_t state) {
    s_led_mgr.state = state;
}

led_system_state_t app_led_get_state(void) {
    return s_led_mgr.state;
}
