#ifndef XL9555_COMPONENT_H
#define XL9555_COMPONENT_H

#include "esp_err.h"
#include <stdbool.h>
#include <stdint.h>
#include "i2c_bus_component.h"
#include "xl9555_pins.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct xl9555_dev xl9555_dev_t;

typedef struct {
    i2c_bus_dev_t *i2c_bus;
    uint8_t        i2c_addr;
    int            int_gpio;
    uint16_t       default_dir;  /* 0=output, 1=input */
    uint16_t       default_out;
} xl9555_config_t;

typedef struct {
    const char *name;
    esp_err_t (*init)(xl9555_dev_t *dev);
    esp_err_t (*deinit)(xl9555_dev_t *dev);
    esp_err_t (*read_ports)(xl9555_dev_t *dev, uint16_t *port_val);
    esp_err_t (*write_ports)(xl9555_dev_t *dev, uint16_t port_val);
    bool      (*pin_read)(xl9555_dev_t *dev, uint16_t pin_mask);
    esp_err_t (*pin_write)(xl9555_dev_t *dev, uint16_t pin_mask, bool val);
    esp_err_t (*set_direction)(xl9555_dev_t *dev, uint16_t dir_mask);
    esp_err_t (*set_polarity)(xl9555_dev_t *dev, uint16_t pol_mask);
} xl9555_driver_iface_t;

struct xl9555_dev {
    const xl9555_driver_iface_t *driver;
    void *driver_ctx;
    bool initialized;
};

xl9555_dev_t* xl9555_create(const xl9555_config_t *cfg);
void xl9555_destroy(xl9555_dev_t *dev);
int xl9555_get_int_gpio(xl9555_dev_t *dev);

static inline esp_err_t xl9555_init(xl9555_dev_t *dev) {
    return (dev && dev->driver) ? dev->driver->init(dev) : ESP_ERR_INVALID_ARG;
}

static inline esp_err_t xl9555_deinit(xl9555_dev_t *dev) {
    return (dev && dev->driver && dev->driver->deinit) ? dev->driver->deinit(dev) : ESP_ERR_INVALID_ARG;
}

static inline esp_err_t xl9555_read_ports(xl9555_dev_t *dev, uint16_t *port_val) {
    return (dev && dev->driver && dev->driver->read_ports) ? dev->driver->read_ports(dev, port_val) : ESP_ERR_INVALID_ARG;
}

static inline esp_err_t xl9555_write_ports(xl9555_dev_t *dev, uint16_t port_val) {
    return (dev && dev->driver && dev->driver->write_ports) ? dev->driver->write_ports(dev, port_val) : ESP_ERR_INVALID_ARG;
}

static inline bool xl9555_pin_read(xl9555_dev_t *dev, uint16_t pin) {
    return (dev && dev->driver && dev->driver->pin_read) ? dev->driver->pin_read(dev, pin) : false;
}

static inline esp_err_t xl9555_pin_write(xl9555_dev_t *dev, uint16_t pin, bool val) {
    return (dev && dev->driver && dev->driver->pin_write) ? dev->driver->pin_write(dev, pin, val) : ESP_ERR_INVALID_ARG;
}

/* LEDR: 低电平点亮，高电平熄灭 */
static inline esp_err_t xl9555_syserr_led_set(xl9555_dev_t *dev, bool on) {
    return xl9555_pin_write(dev, XL9555_P10_LEDR, !on);
}

#ifdef __cplusplus
}
#endif

#endif
