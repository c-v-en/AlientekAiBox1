/*
 * SmartConfig 应用入口
 */

#include "app_init.h"

void app_main(void) {
    /* 创建高优先级初始化任务 */
    xTaskCreatePinnedToCore(app_init_task, "app_init", 4096, NULL, 5, NULL, 1);
}
