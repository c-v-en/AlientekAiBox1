#ifndef XL9555_EVENT_H
#define XL9555_EVENT_H

#include "xl9555_component.h"
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

esp_err_t xl9555_key_event_start(xl9555_dev_t *dev);
void xl9555_key_event_stop(void);

esp_err_t xl9555_syserr_led_auto_register(xl9555_dev_t *dev);
void xl9555_syserr_led_auto_unregister(void);

#ifdef __cplusplus
}
#endif

#endif
