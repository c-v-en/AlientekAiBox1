/*
 * 应用初始化模块
 */

#ifndef APP_INIT_H
#define APP_INIT_H

#include "freertos/FreeRTOS.h"

/**
 * @brief 应用初始化任务
 * 负责依次初始化所有外设，完成后自我删除
 */
void app_init_task(void *pvParameters);

#endif
