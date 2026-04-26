/*
 * MQTT 客户端管理器实现
 * 阿里云私有部署 EMQX (MQTTS / 8883)
 * 运行于 Core0，与 WiFi 同核
 */

#include "app_mqtt.h"
#include "app_event.h"
#include "app_wifi.h"
#include "mqtt_client.h"
#include "esp_log.h"
#include "esp_crt_bundle.h"
#include <string.h>

#define TAG "app_mqtt"

/* ==================== 用户配置区（请按实际部署修改） ==================== */
#ifndef MQTT_BROKER_URI
#define MQTT_BROKER_URI         "mqtt://8.138.2.31:1883"
#endif

#ifndef MQTT_USERNAME
#define MQTT_USERNAME           "Device_1"
#endif

#ifndef MQTT_PASSWORD
#define MQTT_PASSWORD           "1234"
#endif

#ifndef MQTT_CLIENT_ID
#define MQTT_CLIENT_ID          NULL    /* NULL 则使用默认 ESP32_%CHIPID% */
#endif

#ifndef MQTT_KEEPALIVE
#define MQTT_KEEPALIVE          60
#endif

#ifndef MQTT_LWT_TOPIC
#define MQTT_LWT_TOPIC          "device/status"
#endif

#ifndef MQTT_LWT_MSG
#define MQTT_LWT_MSG            "offline"
#endif

#ifndef MQTT_LWT_QOS
#define MQTT_LWT_QOS            1
#endif

#ifndef MQTT_LWT_RETAIN
#define MQTT_LWT_RETAIN         1
#endif

#ifndef MQTT_CMD_TOPIC
#define MQTT_CMD_TOPIC          "device/cmd"
#endif

#ifndef MQTT_CMD_QOS
#define MQTT_CMD_QOS            1
#endif
/* ======================================================================== */

static esp_mqtt_client_handle_t s_client = NULL;
static app_mqtt_state_t s_state = APP_MQTT_STATE_DISCONNECTED;

/* ==================== 内部函数 ==================== */

static void post_mqtt_event(app_event_id_t event_id)
{
    app_event_post_simple(event_id);
}

static void post_mqtt_data_event(const char *topic, int topic_len,
                                 const char *data, int data_len)
{
    app_event_t event = {
        .id = APP_EVENT_MQTT_DATA,
        .priority = APP_EVENT_PRIO_NORMAL,
        .timestamp = xTaskGetTickCount(),
    };

    int tlen = (topic_len < sizeof(event.data.mqtt.topic)) ? topic_len : (sizeof(event.data.mqtt.topic) - 1);
    memcpy(event.data.mqtt.topic, topic, tlen);
    event.data.mqtt.topic[tlen] = '\0';
    event.data.mqtt.topic_len = tlen;

    int plen = (data_len < sizeof(event.data.mqtt.payload)) ? data_len : (sizeof(event.data.mqtt.payload));
    memcpy(event.data.mqtt.payload, data, plen);
    event.data.mqtt.payload_len = plen;

    app_event_post(APP_EVENT_MQTT_DATA, APP_EVENT_PRIO_NORMAL, &event.data, sizeof(event.data));
}

static void mqtt_event_handler(void *handler_args, esp_event_base_t base,
                               int32_t event_id, void *event_data)
{
    (void)handler_args;
    (void)base;

    esp_mqtt_event_handle_t event = event_data;
    int msg_id;

    switch ((esp_mqtt_event_id_t)event_id) {
    case MQTT_EVENT_CONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
        s_state = APP_MQTT_STATE_CONNECTED;
        post_mqtt_event(APP_EVENT_MQTT_CONNECTED);

        /* 发布在线状态 */
        msg_id = esp_mqtt_client_publish(s_client, MQTT_LWT_TOPIC, "online", 0, MQTT_LWT_QOS, MQTT_LWT_RETAIN);
        ESP_LOGI(TAG, "Publish online, msg_id=%d", msg_id);

        /* 订阅云端指令主题 */
        msg_id = esp_mqtt_client_subscribe(s_client, MQTT_CMD_TOPIC, MQTT_CMD_QOS);
        ESP_LOGI(TAG, "Subscribe [%s], msg_id=%d", MQTT_CMD_TOPIC, msg_id);
        break;

    case MQTT_EVENT_DISCONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
        s_state = APP_MQTT_STATE_DISCONNECTED;
        post_mqtt_event(APP_EVENT_MQTT_DISCONNECTED);
        break;

    case MQTT_EVENT_SUBSCRIBED:
        ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
        break;

    case MQTT_EVENT_UNSUBSCRIBED:
        ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
        break;

    case MQTT_EVENT_PUBLISHED:
        ESP_LOGD(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
        break;

    case MQTT_EVENT_DATA:
        ESP_LOGI(TAG, "MQTT_EVENT_DATA, topic=%.*s, len=%d",
                 event->topic_len, event->topic, event->data_len);
        post_mqtt_data_event(event->topic, event->topic_len,
                             event->data, event->data_len);
        break;

    case MQTT_EVENT_ERROR:
        ESP_LOGW(TAG, "MQTT_EVENT_ERROR");
        s_state = APP_MQTT_STATE_ERROR;
        if (event->error_handle->error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT) {
            ESP_LOGW(TAG, "TLS last err: 0x%x", event->error_handle->esp_tls_last_esp_err);
            ESP_LOGW(TAG, "TLS stack err: 0x%x", event->error_handle->esp_tls_stack_err);
            ESP_LOGW(TAG, "Sock errno: %d (%s)",
                     event->error_handle->esp_transport_sock_errno,
                     strerror(event->error_handle->esp_transport_sock_errno));
        } else if (event->error_handle->error_type == MQTT_ERROR_TYPE_CONNECTION_REFUSED) {
            ESP_LOGW(TAG, "Connection refused, code=%d", event->error_handle->connect_return_code);
        }
        break;

    default:
        ESP_LOGD(TAG, "Other event id:%d", event->event_id);
        break;
    }
}

static void wifi_event_handler(const app_event_t *event, void *user_data)
{
    (void)user_data;

    switch (event->id) {
    case APP_EVENT_WIFI_CONNECTED:
        ESP_LOGI(TAG, "WiFi connected, starting MQTT...");
        app_mqtt_start();
        break;

    case APP_EVENT_WIFI_DISCONNECTED:
        ESP_LOGI(TAG, "WiFi disconnected, stopping MQTT...");
        app_mqtt_stop();
        break;

    default:
        break;
    }
}

/* ==================== 接口实现 ==================== */

esp_err_t app_mqtt_init(void)
{
    ESP_LOGI(TAG, "Initializing MQTT client...");

    if (s_client != NULL) {
        return ESP_OK;
    }

    const esp_mqtt_client_config_t mqtt_cfg = {
        .broker = {
            .address.uri = MQTT_BROKER_URI,
            .verification = {
                .crt_bundle_attach = esp_crt_bundle_attach,
            },
        },
        .credentials = {
            .username = MQTT_USERNAME,
            .client_id = MQTT_CLIENT_ID,
            .authentication.password = MQTT_PASSWORD,
        },
        .session = {
            .keepalive = MQTT_KEEPALIVE,
            .last_will = {
                .topic = MQTT_LWT_TOPIC,
                .msg = MQTT_LWT_MSG,
                .qos = MQTT_LWT_QOS,
                .retain = MQTT_LWT_RETAIN,
            },
        },
    };

    s_client = esp_mqtt_client_init(&mqtt_cfg);
    if (!s_client) {
        ESP_LOGE(TAG, "Failed to init mqtt client");
        return ESP_FAIL;
    }

    ESP_ERROR_CHECK(esp_mqtt_client_register_event(s_client, ESP_EVENT_ANY_ID,
                                                   mqtt_event_handler, NULL));

    /* 监听 WiFi 事件，实现自动启停 */
    ESP_ERROR_CHECK(app_event_register_listener(APP_EVENT_WIFI_CONNECTED, wifi_event_handler, NULL));
    ESP_ERROR_CHECK(app_event_register_listener(APP_EVENT_WIFI_DISCONNECTED, wifi_event_handler, NULL));

    ESP_LOGI(TAG, "MQTT client initialized, broker=%s", MQTT_BROKER_URI);
    return ESP_OK;
}

esp_err_t app_mqtt_start(void)
{
    if (!s_client) {
        ESP_LOGW(TAG, "MQTT client not initialized");
        return ESP_ERR_INVALID_STATE;
    }

    if (s_state == APP_MQTT_STATE_CONNECTED || s_state == APP_MQTT_STATE_CONNECTING) {
        return ESP_OK;
    }

    s_state = APP_MQTT_STATE_CONNECTING;
    ESP_LOGI(TAG, "Starting MQTT connection...");
    return esp_mqtt_client_start(s_client);
}

esp_err_t app_mqtt_stop(void)
{
    if (!s_client) {
        return ESP_OK;
    }

    ESP_LOGI(TAG, "Stopping MQTT client...");
    esp_err_t ret = esp_mqtt_client_stop(s_client);
    s_state = APP_MQTT_STATE_DISCONNECTED;
    return ret;
}

app_mqtt_state_t app_mqtt_get_state(void)
{
    return s_state;
}

esp_err_t app_mqtt_publish(const char *topic, const char *data, size_t len, int qos, int retain)
{
    if (!s_client || s_state != APP_MQTT_STATE_CONNECTED) {
        return ESP_ERR_INVALID_STATE;
    }

    if (!topic || !data) {
        return ESP_ERR_INVALID_ARG;
    }

    /* 使用 enqueue 非阻塞入队，适合跨核调用 */
    int msg_id = esp_mqtt_client_enqueue(s_client, topic, data, (int)len, qos, retain, true);
    if (msg_id == -1) {
        ESP_LOGW(TAG, "Failed to enqueue publish to %s", topic);
        return ESP_FAIL;
    }

    ESP_LOGD(TAG, "Enqueued publish to %s, msg_id=%d, len=%u", topic, msg_id, (unsigned)len);
    return ESP_OK;
}

esp_err_t app_mqtt_subscribe(const char *topic, int qos)
{
    if (!s_client || s_state != APP_MQTT_STATE_CONNECTED) {
        return ESP_ERR_INVALID_STATE;
    }

    if (!topic) {
        return ESP_ERR_INVALID_ARG;
    }

    int msg_id = esp_mqtt_client_subscribe(s_client, topic, qos);
    if (msg_id == -1) {
        ESP_LOGW(TAG, "Failed to subscribe %s", topic);
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "Subscribed %s, msg_id=%d", topic, msg_id);
    return ESP_OK;
}
