#ifndef APP_VERSION_H
#define APP_VERSION_H

#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 固件版本信息结构体
 */
typedef struct {
    const char *firmware_name;   // 固件名称
    const char *version;         // 版本号字符串，如 "1.0.0"
    const char *git_commit;      // Git commit hash
    const char *git_branch;      // Git 分支名
    const char *build_time;      // 编译时间
} app_version_info_t;

/**
 * @brief 获取版本信息（返回静态常量指针，无需释放）
 */
const app_version_info_t* app_version_get_info(void);

/**
 * @brief 打印版本横幅到日志
 * 建议在 app_init 中调用
 */
void app_version_log_banner(void);

/**
 * @brief 获取版本号字符串
 */
const char* app_version_get_string(void);

#ifdef __cplusplus
}
#endif

#endif
