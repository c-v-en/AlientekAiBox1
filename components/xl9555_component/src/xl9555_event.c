#include "xl9555_event.h"
#include "xl9555_pins.h"
#include "app_event.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include "esp_log.h"

#define TAG "xl9555_event"

static QueueHandle_t s_int_queue = NULL;
static xl9555_dev_t *s_xl9555 = NULL;
static TaskHandle_t s_key_task = NULL;

/* ========== INT 中断服务程序 ========== */
static void IRAM_ATTR xl9555_int_isr_handler(void *arg)
{
    uint32_t gpio_num = (uint32_t)arg;
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    xQueueSendFromISR(s_int_queue, &gpio_num, &xHigherPriorityTaskWoken);
    if (xHigherPriorityTaskWoken) {
        portYIELD_FROM_ISR();
    }
}

/* ========== 按键扫描任务 ========== */
static void xl9555_key_task(void *pvParam)
{
    xl9555_dev_t *dev = (xl9555_dev_t *)pvParam;
    uint32_t io_num;
    uint16_t last_key_state = 0xFFFF;
    const uint16_t KEY_MASK = XL9555_P03_KEY1 | XL9555_P04_KEY0;

    while (1) {
        if (xQueueReceive(s_int_queue, &io_num, portMAX_DELAY)) {
            vTaskDelay(pdMS_TO_TICKS(10));  /* 硬件消抖 */

            uint16_t curr = 0;
            if (xl9555_read_ports(dev, &curr) != ESP_OK) {
                continue;
            }

            uint16_t pressed = (~curr) & last_key_state & KEY_MASK;

            if (pressed & XL9555_P04_KEY0) {
                app_event_post_simple(APP_EVENT_KEY0_SHORT);
            }
            if (pressed & XL9555_P03_KEY1) {
                app_event_post_simple(APP_EVENT_KEY1_SHORT);
            }

            last_key_state = curr & KEY_MASK;
        }
    }
}

/* ========== 错误 LED 自动响应 ========== */
static void xl9555_syserr_led_event_handler(const app_event_t *event, void *user_data)
{
    xl9555_dev_t *dev = (xl9555_dev_t *)user_data;
    switch (event->id) {
        case APP_EVENT_WIFI_FAILED:
        case APP_EVENT_PROV_TIMEOUT:
            xl9555_syserr_led_set(dev, true);
            break;
        case APP_EVENT_WIFI_CONNECTED:
            xl9555_syserr_led_set(dev, false);
            break;
        default:
            break;
    }
}

/* ========== 公共 API ========== */
esp_err_t xl9555_key_event_start(xl9555_dev_t *dev)
{
    if (!dev) return ESP_ERR_INVALID_ARG;

    int int_gpio = xl9555_get_int_gpio(dev);
    if (int_gpio < 0) return ESP_ERR_INVALID_STATE;

    s_xl9555 = dev;

    /* 创建 ISR 队列 */
    s_int_queue = xQueueCreate(4, sizeof(uint32_t));
    if (!s_int_queue) return ESP_ERR_NO_MEM;

    /* 配置 INT GPIO */
    gpio_config_t gpio_cfg = {
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_NEGEDGE,
        .pin_bit_mask = 1ULL << int_gpio,
    };
    gpio_config(&gpio_cfg);

    gpio_install_isr_service(0);
    gpio_isr_handler_add(int_gpio, xl9555_int_isr_handler, (void *)(uint32_t)int_gpio);

    /* 启动按键任务 */
    BaseType_t ret = xTaskCreate(xl9555_key_task, "xl9555_key", 2048, dev, 2, &s_key_task);
    if (ret != pdPASS) {
        vQueueDelete(s_int_queue);
        s_int_queue = NULL;
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "Key event task started, INT_GPIO=%d", int_gpio);
    return ESP_OK;
}

void xl9555_key_event_stop(void)
{
    if (s_key_task) {
        vTaskDelete(s_key_task);
        s_key_task = NULL;
    }
    if (s_int_queue) {
        vQueueDelete(s_int_queue);
        s_int_queue = NULL;
    }
    s_xl9555 = NULL;
}

esp_err_t xl9555_syserr_led_auto_register(xl9555_dev_t *dev)
{
    if (!dev) return ESP_ERR_INVALID_ARG;

    esp_err_t ret;
    ret = app_event_register_listener(APP_EVENT_WIFI_FAILED, xl9555_syserr_led_event_handler, dev);
    if (ret != ESP_OK) return ret;

    ret = app_event_register_listener(APP_EVENT_PROV_TIMEOUT, xl9555_syserr_led_event_handler, dev);
    if (ret != ESP_OK) return ret;

    return app_event_register_listener(APP_EVENT_WIFI_CONNECTED, xl9555_syserr_led_event_handler, dev);
}

void xl9555_syserr_led_auto_unregister(void)
{
    app_event_unregister_listener(xl9555_syserr_led_event_handler);
}
