#include "led_driver.h"
#include "xl9555_component.h"
#include "xl9555_pins.h"
#include "esp_err.h"

static esp_err_t xl9555_led_init(led_device_t *dev)
{
    dev->initialized = true;
    return ESP_OK;
}

static esp_err_t xl9555_led_deinit(led_device_t *dev)
{
    dev->initialized = false;
    return ESP_OK;
}

static esp_err_t xl9555_led_on(led_device_t *dev)
{
    xl9555_dev_t *xl = (xl9555_dev_t *)dev->driver_ctx;
    return xl9555_pin_write(xl, XL9555_P10_LEDR, 0);  /* 低电平点亮 */
}

static esp_err_t xl9555_led_off(led_device_t *dev)
{
    xl9555_dev_t *xl = (xl9555_dev_t *)dev->driver_ctx;
    return xl9555_pin_write(xl, XL9555_P10_LEDR, 1);  /* 高电平熄灭 */
}

static esp_err_t xl9555_led_toggle(led_device_t *dev)
{
    xl9555_dev_t *xl = (xl9555_dev_t *)dev->driver_ctx;
    bool curr = xl9555_pin_read(xl, XL9555_P10_LEDR);
    return xl9555_pin_write(xl, XL9555_P10_LEDR, !curr);
}

static bool xl9555_led_is_on(led_device_t *dev)
{
    xl9555_dev_t *xl = (xl9555_dev_t *)dev->driver_ctx;
    return !xl9555_pin_read(xl, XL9555_P10_LEDR);  /* 低电平为 ON */
}

const led_driver_iface_t xl9555_ledr_driver = {
    .name = "xl9555_ledr",
    .init = xl9555_led_init,
    .deinit = xl9555_led_deinit,
    .on = xl9555_led_on,
    .off = xl9555_led_off,
    .toggle = xl9555_led_toggle,
    .is_on = xl9555_led_is_on,
};
