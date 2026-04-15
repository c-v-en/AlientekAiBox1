/*
 * 事件分发层实现
 * 基于优先级的队列实现
 */

#include "app_event.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_log.h"
#include <string.h>

#define TAG "app_event"
#define MAX_LISTENERS 10
#define EVENT_QUEUE_SIZE 32

typedef struct {
    app_event_handler_t handler;
    int event_id;                   /* -1 表示监听所有 */
    void *user_data;
    bool active;
} listener_t;

/* 优先队列节点 */
typedef struct {
    app_event_t event;
    bool valid;
} queue_node_t;

static struct {
    queue_node_t queue[EVENT_QUEUE_SIZE];  /* 循环优先队列 */
    uint8_t head;                          /* 队首索引 */
    uint8_t count;                         /* 当前数量 */
    
    SemaphoreHandle_t mutex;               /* 队列互斥锁 */
    SemaphoreHandle_t notify;              /* 有新事件通知 */
    
    listener_t listeners[MAX_LISTENERS];
    TaskHandle_t task;
    bool running;
} s_ctx;

/* 插入事件到优先队列（按优先级排序） */
static bool queue_insert(const app_event_t *event) {
    if (s_ctx.count >= EVENT_QUEUE_SIZE) {
        return false;
    }
    
    /* 找到插入位置（优先级高的在前，同优先级按时间先后） */
    uint8_t insert_pos = s_ctx.head;
    uint8_t sorted_count = 0;
    
    while (sorted_count < s_ctx.count) {
        queue_node_t *node = &s_ctx.queue[insert_pos % EVENT_QUEUE_SIZE];
        
        /* 当前位置优先级更低，或同优先级但时间更晚，则插入此处 */
        if (node->event.priority < event->priority ||
            (node->event.priority == event->priority && 
             node->event.timestamp > event->timestamp)) {
            break;
        }
        
        insert_pos++;
        sorted_count++;
    }
    
    /* 腾出位置：将insert_pos后的元素后移 */
    uint8_t tail = (s_ctx.head + s_ctx.count) % EVENT_QUEUE_SIZE;
    uint8_t move_pos = tail;
    
    while (move_pos != insert_pos) {
        uint8_t prev = (move_pos + EVENT_QUEUE_SIZE - 1) % EVENT_QUEUE_SIZE;
        s_ctx.queue[move_pos] = s_ctx.queue[prev];
        move_pos = prev;
    }
    
    /* 插入新事件 */
    s_ctx.queue[insert_pos % EVENT_QUEUE_SIZE].event = *event;
    s_ctx.queue[insert_pos % EVENT_QUEUE_SIZE].valid = true;
    s_ctx.count++;
    
    return true;
}

/* 取出队首事件 */
static bool queue_pop(app_event_t *event) {
    if (s_ctx.count == 0) {
        return false;
    }
    
    *event = s_ctx.queue[s_ctx.head].event;
    s_ctx.queue[s_ctx.head].valid = false;
    s_ctx.head = (s_ctx.head + 1) % EVENT_QUEUE_SIZE;
    s_ctx.count--;
    
    return true;
}

static void app_event_task(void *pvParam) {
    app_event_t event;
    
    (void)pvParam;
    
    while (s_ctx.running) {
        /* 等待事件通知 */
        if (xSemaphoreTake(s_ctx.notify, portMAX_DELAY) == pdTRUE) {
            /* 处理队列中所有当前事件 */
            while (s_ctx.count > 0) {
                xSemaphoreTake(s_ctx.mutex, portMAX_DELAY);
                bool has_event = queue_pop(&event);
                xSemaphoreGive(s_ctx.mutex);
                
                if (!has_event) break;
                
                /* 分发到监听者 */
                for (int i = 0; i < MAX_LISTENERS; i++) {
                    if (s_ctx.listeners[i].active) {
                        if (s_ctx.listeners[i].event_id == -1 ||
                            s_ctx.listeners[i].event_id == event.id) {
                            s_ctx.listeners[i].handler(&event, 
                                s_ctx.listeners[i].user_data);
                        }
                    }
                }
            }
        }
    }
    
    vTaskDelete(NULL);
}

esp_err_t app_event_init(void) {
    memset(&s_ctx, 0, sizeof(s_ctx));
    
    s_ctx.mutex = xSemaphoreCreateMutex();
    s_ctx.notify = xSemaphoreCreateBinary();
    
    if (!s_ctx.mutex || !s_ctx.notify) {
        return ESP_ERR_NO_MEM;
    }
    
    s_ctx.running = true;
    
    if (xTaskCreate(app_event_task, "app_event", 4096, NULL, 4, &s_ctx.task) != pdPASS) {
        return ESP_FAIL;
    }
    
    ESP_LOGI(TAG, "Event system initialized");
    return ESP_OK;
}

esp_err_t app_event_register_listener(app_event_id_t event_id,
                                       app_event_handler_t handler,
                                       void *user_data) {
    if (!handler) return ESP_ERR_INVALID_ARG;
    
    xSemaphoreTake(s_ctx.mutex, portMAX_DELAY);
    
    for (int i = 0; i < MAX_LISTENERS; i++) {
        if (!s_ctx.listeners[i].active) {
            s_ctx.listeners[i].handler = handler;
            s_ctx.listeners[i].event_id = event_id;
            s_ctx.listeners[i].user_data = user_data;
            s_ctx.listeners[i].active = true;
            
            xSemaphoreGive(s_ctx.mutex);
            ESP_LOGD(TAG, "Listener registered for event %d", event_id);
            return ESP_OK;
        }
    }
    
    xSemaphoreGive(s_ctx.mutex);
    return ESP_ERR_NO_MEM;
}

esp_err_t app_event_unregister_listener(app_event_handler_t handler) {
    xSemaphoreTake(s_ctx.mutex, portMAX_DELAY);
    
    for (int i = 0; i < MAX_LISTENERS; i++) {
        if (s_ctx.listeners[i].active && 
            s_ctx.listeners[i].handler == handler) {
            s_ctx.listeners[i].active = false;
            xSemaphoreGive(s_ctx.mutex);
            return ESP_OK;
        }
    }
    
    xSemaphoreGive(s_ctx.mutex);
    return ESP_ERR_NOT_FOUND;
}

esp_err_t app_event_post(app_event_id_t event_id,
                          app_event_priority_t priority,
                          const void *data,
                          size_t data_size) {
    if (event_id >= APP_EVENT_MAX) return ESP_ERR_INVALID_ARG;
    
    app_event_t event = {
        .id = event_id,
        .priority = priority,
        .timestamp = xTaskGetTickCount(),
    };
    
    /* 复制数据 */
    if (data && data_size > 0) {
        memcpy(&event.data, data, (data_size > sizeof(event.data)) ? 
                                   sizeof(event.data) : data_size);
    }
    
    xSemaphoreTake(s_ctx.mutex, portMAX_DELAY);
    
    if (!queue_insert(&event)) {
        xSemaphoreGive(s_ctx.mutex);
        ESP_LOGW(TAG, "Event queue full, event %d dropped", event_id);
        return ESP_FAIL;
    }
    
    xSemaphoreGive(s_ctx.mutex);
    xSemaphoreGive(s_ctx.notify);  /* 通知任务处理 */
    
    return ESP_OK;
}

esp_err_t app_event_post_simple(app_event_id_t event_id) {
    /* 根据事件类型自动分配优先级 */
    app_event_priority_t prio = APP_EVENT_PRIO_NORMAL;
    
    switch (event_id) {
        case APP_EVENT_WIFI_CONNECTING:
        case APP_EVENT_WIFI_CONNECTED:
        case APP_EVENT_WIFI_DISCONNECTED:
            prio = APP_EVENT_PRIO_CRITICAL;
            break;
        case APP_EVENT_WIFI_SCANNING:
        case APP_EVENT_WIFI_FAILED:
            prio = APP_EVENT_PRIO_HIGH;
            break;
        case APP_EVENT_PROV_TIMEOUT:
        case APP_EVENT_PROV_STOPPED:
            prio = APP_EVENT_PRIO_HIGH;
            break;
        case APP_EVENT_VERSION_INFO:
            prio = APP_EVENT_PRIO_LOW;
            break;
        default:
            prio = APP_EVENT_PRIO_NORMAL;
            break;
    }
    
    return app_event_post(event_id, prio, NULL, 0);
}

void app_event_flush(void) {
    xSemaphoreTake(s_ctx.mutex, portMAX_DELAY);
    s_ctx.head = 0;
    s_ctx.count = 0;
    memset(s_ctx.queue, 0, sizeof(s_ctx.queue));
    xSemaphoreGive(s_ctx.mutex);
}
