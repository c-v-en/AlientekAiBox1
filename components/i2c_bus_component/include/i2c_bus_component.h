#ifndef I2C_BUS_COMPONENT_H
#define I2C_BUS_COMPONENT_H

#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct i2c_bus_dev i2c_bus_dev_t;

typedef struct {
    const char *name;
    esp_err_t (*init)(i2c_bus_dev_t *bus);
    esp_err_t (*deinit)(i2c_bus_dev_t *bus);
    esp_err_t (*read_reg)(i2c_bus_dev_t *bus, uint8_t dev_addr,
                          uint8_t reg, uint8_t *data, size_t len);
    esp_err_t (*write_reg)(i2c_bus_dev_t *bus, uint8_t dev_addr,
                           uint8_t reg, const uint8_t *data, size_t len);
    esp_err_t (*bus_recovery)(i2c_bus_dev_t *bus);
} i2c_bus_iface_t;

struct i2c_bus_dev {
    const i2c_bus_iface_t *driver;
    void *driver_ctx;
    bool initialized;
    SemaphoreHandle_t mutex;
};

/* 工厂方法 */
i2c_bus_dev_t* i2c_bus_create_esp32(int scl_gpio, int sda_gpio, uint32_t freq);

/* 内联包装 */
static inline esp_err_t i2c_bus_init(i2c_bus_dev_t *bus) {
    return (bus && bus->driver) ? bus->driver->init(bus) : ESP_ERR_INVALID_ARG;
}

static inline esp_err_t i2c_bus_deinit(i2c_bus_dev_t *bus) {
    return (bus && bus->driver && bus->driver->deinit) ? bus->driver->deinit(bus) : ESP_ERR_INVALID_ARG;
}

static inline esp_err_t i2c_bus_read_reg(i2c_bus_dev_t *bus, uint8_t addr,
                                         uint8_t reg, uint8_t *data, size_t len) {
    if (!bus || !bus->driver || !bus->driver->read_reg) return ESP_ERR_INVALID_ARG;
    xSemaphoreTake(bus->mutex, portMAX_DELAY);
    esp_err_t ret = bus->driver->read_reg(bus, addr, reg, data, len);
    xSemaphoreGive(bus->mutex);
    return ret;
}

static inline esp_err_t i2c_bus_write_reg(i2c_bus_dev_t *bus, uint8_t addr,
                                          uint8_t reg, const uint8_t *data, size_t len) {
    if (!bus || !bus->driver || !bus->driver->write_reg) return ESP_ERR_INVALID_ARG;
    xSemaphoreTake(bus->mutex, portMAX_DELAY);
    esp_err_t ret = bus->driver->write_reg(bus, addr, reg, data, len);
    xSemaphoreGive(bus->mutex);
    return ret;
}

static inline esp_err_t i2c_bus_recovery(i2c_bus_dev_t *bus) {
    if (!bus || !bus->driver || !bus->driver->bus_recovery) return ESP_ERR_INVALID_ARG;
    xSemaphoreTake(bus->mutex, portMAX_DELAY);
    esp_err_t ret = bus->driver->bus_recovery(bus);
    xSemaphoreGive(bus->mutex);
    return ret;
}

#ifdef __cplusplus
}
#endif

#endif
