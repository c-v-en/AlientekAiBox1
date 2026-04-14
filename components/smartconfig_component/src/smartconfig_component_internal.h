/*
 * SmartConfig Component Internal API
 * 
 * 这是内部头文件，仅供组件内部实现使用
 * 驱动实现需要包含此文件来调用组件内部函数
 * 应用程序不应直接包含或使用此文件
 */

#ifndef SMARTCONFIG_COMPONENT_INTERNAL_H
#define SMARTCONFIG_COMPONENT_INTERNAL_H

#include "smartconfig_component.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 驱动获取SSID/PSWD后调用此函数通知组件
 * 
 * @param self 组件实例
 * @param ssid SSID (32字节)
 * @param password 密码 (64字节)
 * @param bssid AP的BSSID (6字节，可为NULL)
 * @param channel WiFi信道
 */
void sc_internal_on_got_ssid_pswd(sc_component_t *self, 
                                   const uint8_t *ssid, 
                                   const uint8_t *password,
                                   const uint8_t *bssid,
                                   uint8_t channel);

/**
 * @brief 驱动状态变化时调用
 * 
 * @param self 组件实例
 * @param status 当前状态
 */
void sc_internal_on_driver_status(sc_component_t *self, sc_status_t status);

/**
 * @brief WiFi连接成功时调用
 * 
 * @param self 组件实例
 */
void sc_internal_on_wifi_connected(sc_component_t *self);

/**
 * @brief WiFi断开时调用
 * 
 * @param self 组件实例
 */
void sc_internal_on_wifi_disconnected(sc_component_t *self);

#ifdef __cplusplus
}
#endif

#endif /* SMARTCONFIG_COMPONENT_INTERNAL_H */
