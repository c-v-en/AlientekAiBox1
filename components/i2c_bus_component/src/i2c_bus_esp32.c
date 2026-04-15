#include "i2c_bus_component.h"
#include "driver/i2c_master.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_rom_sys.h"
#include <stdlib.h>
#include <string.h>

static const char *TAG = "i2c_bus_esp32";

typedef struct {
    int scl_gpio;
    int sda_gpio;
    uint32_t freq;
    uint8_t dev_addr;
    bool initialized;
    i2c_master_bus_handle_t bus_handle;
    i2c_master_dev_handle_t dev_handle;
} i2c_esp32_ctx_t;

static esp_err_t i2c_esp32_bus_recovery(i2c_bus_dev_t *bus);
static esp_err_t i2c_esp32_ensure_device(i2c_esp32_ctx_t *ctx, uint8_t dev_addr);

static esp_err_t i2c_esp32_ensure_device(i2c_esp32_ctx_t *ctx, uint8_t dev_addr)
{
    if (ctx->dev_handle && ctx->dev_addr == dev_addr) {
        return ESP_OK;
    }
    if (ctx->dev_handle) {
        i2c_master_bus_rm_device(ctx->dev_handle);
        ctx->dev_handle = NULL;
    }
    ctx->dev_addr = dev_addr;
    i2c_device_config_t dev_cfg = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .scl_speed_hz = ctx->freq,
        .device_address = dev_addr,
    };
    return i2c_master_bus_add_device(ctx->bus_handle, &dev_cfg, &ctx->dev_handle);
}

static esp_err_t i2c_esp32_init(i2c_bus_dev_t *bus)
{
    i2c_esp32_ctx_t *ctx = (i2c_esp32_ctx_t *)bus->driver_ctx;
    if (!ctx) return ESP_ERR_INVALID_ARG;

    i2c_master_bus_config_t bus_cfg = {
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .i2c_port = I2C_NUM_0,
        .scl_io_num = ctx->scl_gpio,
        .sda_io_num = ctx->sda_gpio,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = true,
    };

    ESP_ERROR_CHECK(i2c_new_master_bus(&bus_cfg, &ctx->bus_handle));

    ctx->initialized = true;
    ESP_LOGI(TAG, "I2C bus init OK (SCL=%d, SDA=%d, freq=%luHz)", ctx->scl_gpio, ctx->sda_gpio, ctx->freq);
    return ESP_OK;
}

static esp_err_t i2c_esp32_deinit(i2c_bus_dev_t *bus)
{
    i2c_esp32_ctx_t *ctx = (i2c_esp32_ctx_t *)bus->driver_ctx;
    if (!ctx) return ESP_OK;

    if (ctx->dev_handle) {
        i2c_master_bus_rm_device(ctx->dev_handle);
        ctx->dev_handle = NULL;
    }
    if (ctx->bus_handle) {
        i2c_del_master_bus(ctx->bus_handle);
        ctx->bus_handle = NULL;
    }
    ctx->initialized = false;
    return ESP_OK;
}

static esp_err_t i2c_esp32_read_reg(i2c_bus_dev_t *bus, uint8_t dev_addr,
                                    uint8_t reg, uint8_t *data, size_t len)
{
    i2c_esp32_ctx_t *ctx = (i2c_esp32_ctx_t *)bus->driver_ctx;
    if (!ctx || !ctx->bus_handle) return ESP_ERR_INVALID_STATE;

    ESP_ERROR_CHECK(i2c_esp32_ensure_device(ctx, dev_addr));

    esp_err_t ret = i2c_master_transmit_receive(ctx->dev_handle, &reg, 1, data, len, -1);
    if (ret != ESP_OK) {
        ESP_LOGW(TAG, "Read failed: %s, attempting bus recovery...", esp_err_to_name(ret));
        ret = i2c_esp32_bus_recovery(bus);
        if (ret == ESP_OK) {
            ESP_ERROR_CHECK(i2c_esp32_ensure_device(ctx, dev_addr));
            ret = i2c_master_transmit_receive(ctx->dev_handle, &reg, 1, data, len, -1);
        }
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "Read failed after recovery");
            return ESP_ERR_TIMEOUT;
        }
    }
    return ESP_OK;
}

static esp_err_t i2c_esp32_write_reg(i2c_bus_dev_t *bus, uint8_t dev_addr,
                                     uint8_t reg, const uint8_t *data, size_t len)
{
    i2c_esp32_ctx_t *ctx = (i2c_esp32_ctx_t *)bus->driver_ctx;
    if (!ctx || !ctx->bus_handle) return ESP_ERR_INVALID_STATE;

    ESP_ERROR_CHECK(i2c_esp32_ensure_device(ctx, dev_addr));

    uint8_t *buf = malloc(1 + len);
    if (!buf) return ESP_ERR_NO_MEM;

    buf[0] = reg;
    memcpy(buf + 1, data, len);

    esp_err_t ret = i2c_master_transmit(ctx->dev_handle, buf, len + 1, -1);
    free(buf);

    if (ret != ESP_OK) {
        ESP_LOGW(TAG, "Write failed: %s, attempting bus recovery...", esp_err_to_name(ret));
        ret = i2c_esp32_bus_recovery(bus);
        if (ret == ESP_OK) {
            ESP_ERROR_CHECK(i2c_esp32_ensure_device(ctx, dev_addr));
            buf = malloc(1 + len);
            if (buf) {
                buf[0] = reg;
                memcpy(buf + 1, data, len);
                ret = i2c_master_transmit(ctx->dev_handle, buf, len + 1, -1);
                free(buf);
            } else {
                return ESP_ERR_NO_MEM;
            }
        }
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "Write failed after recovery");
            return ESP_ERR_TIMEOUT;
        }
    }
    return ESP_OK;
}

static esp_err_t i2c_esp32_bus_recovery(i2c_bus_dev_t *bus)
{
    i2c_esp32_ctx_t *ctx = (i2c_esp32_ctx_t *)bus->driver_ctx;
    if (!ctx) return ESP_ERR_INVALID_ARG;

    ESP_LOGW(TAG, "Executing I2C bus recovery...");

    /* 1. 删除当前设备句柄和总线 */
    if (ctx->dev_handle) {
        i2c_master_bus_rm_device(ctx->dev_handle);
        ctx->dev_handle = NULL;
    }
    if (ctx->bus_handle) {
        i2c_del_master_bus(ctx->bus_handle);
        ctx->bus_handle = NULL;
    }

    /* 2. 将 SCL/SDA 配置为 GPIO 开漏输出 */
    gpio_set_direction(ctx->scl_gpio, GPIO_MODE_OUTPUT_OD);
    gpio_set_direction(ctx->sda_gpio, GPIO_MODE_OUTPUT_OD);
    gpio_set_level(ctx->sda_gpio, 1);

    /* 3. 发送 9~16 个 SCL 时钟脉冲，检测 SDA 释放 */
    for (int i = 0; i < 16; i++) {
        gpio_set_level(ctx->scl_gpio, 1);
        esp_rom_delay_us(5);
        gpio_set_level(ctx->scl_gpio, 0);
        esp_rom_delay_us(5);
        if (gpio_get_level(ctx->sda_gpio) == 1) {
            break;
        }
    }

    /* 4. 发送 STOP 条件 */
    gpio_set_level(ctx->scl_gpio, 1);
    esp_rom_delay_us(5);
    gpio_set_level(ctx->sda_gpio, 1);
    esp_rom_delay_us(5);

    /* 5. 重新初始化 I2C Master 总线 */
    i2c_master_bus_config_t bus_cfg = {
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .i2c_port = I2C_NUM_0,
        .scl_io_num = ctx->scl_gpio,
        .sda_io_num = ctx->sda_gpio,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = true,
    };

    esp_err_t ret = i2c_new_master_bus(&bus_cfg, &ctx->bus_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Bus recovery failed: unable to create bus");
        return ret;
    }

    ESP_LOGI(TAG, "Bus recovery success");
    return ESP_OK;
}

static const i2c_bus_iface_t i2c_bus_esp32_iface = {
    .name = "esp32_i2c_master",
    .init = i2c_esp32_init,
    .deinit = i2c_esp32_deinit,
    .read_reg = i2c_esp32_read_reg,
    .write_reg = i2c_esp32_write_reg,
    .bus_recovery = i2c_esp32_bus_recovery,
};

i2c_bus_dev_t* i2c_bus_create_esp32(int scl_gpio, int sda_gpio, uint32_t freq)
{
    i2c_esp32_ctx_t *ctx = calloc(1, sizeof(i2c_esp32_ctx_t));
    if (!ctx) return NULL;

    ctx->scl_gpio = scl_gpio;
    ctx->sda_gpio = sda_gpio;
    ctx->freq = freq;
    ctx->dev_addr = 0x00;

    i2c_bus_dev_t *bus = malloc(sizeof(i2c_bus_dev_t));
    if (!bus) {
        free(ctx);
        return NULL;
    }

    bus->driver = &i2c_bus_esp32_iface;
    bus->driver_ctx = ctx;
    bus->initialized = false;

    return bus;
}
