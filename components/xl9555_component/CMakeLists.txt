idf_component_register(
    SRCS
        "src/xl9555_component.c"
        "src/xl9555_event.c"
        # "src/xl9555_gpio_adapter.c"  /* 预留：当需要将LEDR接入app_led.c时启用 */
    INCLUDE_DIRS "include"
    REQUIRES
        i2c_bus_component
        app_event_component
        driver
        freertos
        esp_driver_gpio
)
