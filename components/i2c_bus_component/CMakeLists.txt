idf_component_register(
    SRCS "src/i2c_bus_esp32.c"
    INCLUDE_DIRS "include"
    REQUIRES
        driver
        esp_driver_i2c
        esp_driver_gpio
)
