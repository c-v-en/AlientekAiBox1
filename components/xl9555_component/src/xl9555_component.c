#include "xl9555_component.h"
#include "xl9555_pins.h"
#include "esp_log.h"
#include <stdlib.h>
#include <string.h>

static const char *TAG = "xl9555";

#define XL9555_INPUT_PORT0_REG      0
#define XL9555_INPUT_PORT1_REG      1
#define XL9555_OUTPUT_PORT0_REG     2
#define XL9555_OUTPUT_PORT1_REG     3
#define XL9555_INVERSION_PORT0_REG  4
#define XL9555_INVERSION_PORT1_REG  5
#define XL9555_CONFIG_PORT0_REG     6
#define XL9555_CONFIG_PORT1_REG     7

typedef struct {
    i2c_bus_dev_t *i2c_bus;
    uint8_t        i2c_addr;
    int            int_gpio;
    uint16_t       default_dir;
    uint16_t       default_out;
} xl9555_ctx_t;

static esp_err_t xl9555_core_init(xl9555_dev_t *dev)
{
    xl9555_ctx_t *ctx = (xl9555_ctx_t *)dev->driver_ctx;
    if (!ctx) return ESP_ERR_INVALID_ARG;

    uint8_t data[2];

    /* 极性反转：默认不反转 */
    data[0] = 0x00;
    data[1] = 0x00;
    ESP_ERROR_CHECK(i2c_bus_write_reg(ctx->i2c_bus, ctx->i2c_addr,
                                      XL9555_INVERSION_PORT0_REG, data, 2));

    /* 方向配置 */
    data[0] = (uint8_t)(ctx->default_dir & 0xFF);
    data[1] = (uint8_t)(ctx->default_dir >> 8);
    ESP_ERROR_CHECK(i2c_bus_write_reg(ctx->i2c_bus, ctx->i2c_addr,
                                      XL9555_CONFIG_PORT0_REG, data, 2));

    /* 默认输出状态 */
    data[0] = (uint8_t)(ctx->default_out & 0xFF);
    data[1] = (uint8_t)(ctx->default_out >> 8);
    ESP_ERROR_CHECK(i2c_bus_write_reg(ctx->i2c_bus, ctx->i2c_addr,
                                      XL9555_OUTPUT_PORT0_REG, data, 2));

    dev->initialized = true;
    ESP_LOGI(TAG, "XL9555 init OK, dir=0x%04X, out=0x%04X", ctx->default_dir, ctx->default_out);
    return ESP_OK;
}

static esp_err_t xl9555_core_deinit(xl9555_dev_t *dev)
{
    if (dev) dev->initialized = false;
    return ESP_OK;
}

static esp_err_t xl9555_core_read_ports(xl9555_dev_t *dev, uint16_t *port_val)
{
    xl9555_ctx_t *ctx = (xl9555_ctx_t *)dev->driver_ctx;
    if (!ctx || !port_val) return ESP_ERR_INVALID_ARG;

    uint8_t data[2];
    esp_err_t ret = i2c_bus_read_reg(ctx->i2c_bus, ctx->i2c_addr,
                                     XL9555_INPUT_PORT0_REG, data, 2);
    if (ret == ESP_OK) {
        *port_val = ((uint16_t)data[1] << 8) | data[0];
    }
    return ret;
}

static esp_err_t xl9555_core_write_ports(xl9555_dev_t *dev, uint16_t port_val)
{
    xl9555_ctx_t *ctx = (xl9555_ctx_t *)dev->driver_ctx;
    if (!ctx) return ESP_ERR_INVALID_ARG;

    uint8_t data[2];
    data[0] = (uint8_t)(port_val & 0xFF);
    data[1] = (uint8_t)(port_val >> 8);
    return i2c_bus_write_reg(ctx->i2c_bus, ctx->i2c_addr,
                             XL9555_OUTPUT_PORT0_REG, data, 2);
}

static bool xl9555_core_pin_read(xl9555_dev_t *dev, uint16_t pin_mask)
{
    uint16_t port_val = 0;
    if (xl9555_core_read_ports(dev, &port_val) != ESP_OK) return false;
    return (port_val & pin_mask) ? true : false;
}

static esp_err_t xl9555_core_pin_write(xl9555_dev_t *dev, uint16_t pin_mask, bool val)
{
    uint16_t port_val = 0;
    esp_err_t ret = xl9555_core_read_ports(dev, &port_val);
    if (ret != ESP_OK) return ret;

    if (val) {
        port_val |= pin_mask;
    } else {
        port_val &= ~pin_mask;
    }

    return xl9555_core_write_ports(dev, port_val);
}

static esp_err_t xl9555_core_set_direction(xl9555_dev_t *dev, uint16_t dir_mask)
{
    xl9555_ctx_t *ctx = (xl9555_ctx_t *)dev->driver_ctx;
    if (!ctx) return ESP_ERR_INVALID_ARG;

    uint8_t data[2];
    data[0] = (uint8_t)(dir_mask & 0xFF);
    data[1] = (uint8_t)(dir_mask >> 8);
    return i2c_bus_write_reg(ctx->i2c_bus, ctx->i2c_addr,
                             XL9555_CONFIG_PORT0_REG, data, 2);
}

static esp_err_t xl9555_core_set_polarity(xl9555_dev_t *dev, uint16_t pol_mask)
{
    xl9555_ctx_t *ctx = (xl9555_ctx_t *)dev->driver_ctx;
    if (!ctx) return ESP_ERR_INVALID_ARG;

    uint8_t data[2];
    data[0] = (uint8_t)(pol_mask & 0xFF);
    data[1] = (uint8_t)(pol_mask >> 8);
    return i2c_bus_write_reg(ctx->i2c_bus, ctx->i2c_addr,
                             XL9555_INVERSION_PORT0_REG, data, 2);
}

static const xl9555_driver_iface_t xl9555_default_driver = {
    .name = "xl9555",
    .init = xl9555_core_init,
    .deinit = xl9555_core_deinit,
    .read_ports = xl9555_core_read_ports,
    .write_ports = xl9555_core_write_ports,
    .pin_read = xl9555_core_pin_read,
    .pin_write = xl9555_core_pin_write,
    .set_direction = xl9555_core_set_direction,
    .set_polarity = xl9555_core_set_polarity,
};

xl9555_dev_t* xl9555_create(const xl9555_config_t *cfg)
{
    if (!cfg || !cfg->i2c_bus) return NULL;

    xl9555_ctx_t *ctx = calloc(1, sizeof(xl9555_ctx_t));
    if (!ctx) return NULL;

    ctx->i2c_bus = cfg->i2c_bus;
    ctx->i2c_addr = cfg->i2c_addr;
    ctx->int_gpio = cfg->int_gpio;
    ctx->default_dir = cfg->default_dir;
    ctx->default_out = cfg->default_out;

    xl9555_dev_t *dev = malloc(sizeof(xl9555_dev_t));
    if (!dev) {
        free(ctx);
        return NULL;
    }

    dev->driver = &xl9555_default_driver;
    dev->driver_ctx = ctx;
    dev->initialized = false;

    return dev;
}

void xl9555_destroy(xl9555_dev_t *dev)
{
    if (dev) {
        xl9555_deinit(dev);
        if (dev->driver_ctx) {
            free(dev->driver_ctx);
        }
        free(dev);
    }
}

int xl9555_get_int_gpio(xl9555_dev_t *dev)
{
    xl9555_ctx_t *ctx = (xl9555_ctx_t *)dev->driver_ctx;
    return ctx ? ctx->int_gpio : -1;
}
