/*
 * Copyright (c) 2006-2025, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2025-01-14     Claude       WiFi管理模块
 */

#ifndef __WIFI_MANAGER_H__
#define __WIFI_MANAGER_H__

#include <rtthread.h>

/* WiFi连接状态 */
typedef enum {
    WIFI_STATUS_DISCONNECTED = 0,
    WIFI_STATUS_CONNECTING,
    WIFI_STATUS_CONNECTED,
    WIFI_STATUS_CONNECT_FAILED
} wifi_status_t;

/**
 * @brief 初始化WiFi管理模块
 * @return 0: 成功, -1: 失败
 */
int wifi_manager_init(void);

/**
 * @brief 连接到指定的WiFi AP
 * @param ssid WiFi SSID
 * @param password WiFi密码
 * @return 0: 成功, -1: 失败
 */
int wifi_connect(const char *ssid, const char *password);

/**
 * @brief 断开WiFi连接
 * @return 0: 成功, -1: 失败
 */
int wifi_disconnect(void);

/**
 * @brief 获取WiFi连接状态
 * @return wifi_status_t 连接状态
 */
wifi_status_t wifi_get_status(void);

/**
 * @brief 获取本地IP地址
 * @param ip_buf 存储IP地址的缓冲区
 * @param buf_len 缓冲区长度
 * @return 0: 成功, -1: 失败
 */
int wifi_get_ip(char *ip_buf, int buf_len);

/**
 * @brief 等待WiFi连接成功
 * @param timeout_ms 超时时间(毫秒)
 * @return 0: 成功, -1: 超时或失败
 */
int wifi_wait_ready(int timeout_ms);

#endif /* __WIFI_MANAGER_H__ */
