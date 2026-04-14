#include "app_version.h"
#include "version_info.h"
#include "esp_log.h"

static const char *TAG = "app_version";

static const app_version_info_t s_version_info = {
    .firmware_name = "AlientekAiBox1",
    .version       = APP_VERSION_MAJOR "." APP_VERSION_MINOR "." APP_VERSION_PATCH,
    .git_commit    = APP_GIT_COMMIT,
    .git_branch    = APP_GIT_BRANCH,
    .build_time    = APP_BUILD_TIME,
};

const app_version_info_t* app_version_get_info(void)
{
    return &s_version_info;
}

const char* app_version_get_string(void)
{
    return s_version_info.version;
}

void app_version_log_banner(void)
{
    ESP_LOGI(TAG, "========================================");
    ESP_LOGI(TAG, "  Firmware : %s", s_version_info.firmware_name);
    ESP_LOGI(TAG, "  Version  : %s", s_version_info.version);
    ESP_LOGI(TAG, "  Git      : %s@%s", s_version_info.git_branch, s_version_info.git_commit);
    ESP_LOGI(TAG, "  Build    : %s", s_version_info.build_time);
    ESP_LOGI(TAG, "========================================");
}
