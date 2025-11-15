/*
 * Copyright (c) 2006-2025, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2025-01-14     Claude       WiFi管理模块实现
 */

#include "wifi_manager.h"
#include <rtthread.h>
#include <wlan_mgnt.h>
#include <wlan_prot.h>
#include <wlan_cfg.h>

/* 包含lwIP头文件 */
#include <lwip/netif.h>
#include <lwip/inet.h>
#include <netdev.h>

#define DBG_TAG "wifi.mgr"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

static wifi_status_t g_wifi_status = WIFI_STATUS_DISCONNECTED;
static struct rt_event wifi_event;

/**
 * @brief WiFi事件回调函数
 */
static void wifi_event_handler(int event, struct rt_wlan_buff *buff, void *parameter)
{
    switch (event)
    {
    case RT_WLAN_EVT_READY:
        LOG_I("WiFi ready");
        g_wifi_status = WIFI_STATUS_CONNECTED;
        rt_event_send(&wifi_event, (1 << 0));
        break;

    case RT_WLAN_EVT_STA_CONNECTED:
        LOG_I("WiFi STA connected");
        break;

    case RT_WLAN_EVT_STA_DISCONNECTED:
        LOG_W("WiFi STA disconnected");
        g_wifi_status = WIFI_STATUS_DISCONNECTED;
        break;

    case RT_WLAN_EVT_STA_CONNECTED_FAIL:
        LOG_E("WiFi STA connect failed");
        g_wifi_status = WIFI_STATUS_CONNECT_FAILED;
        break;

    case RT_WLAN_EVT_AP_START:
        LOG_I("WiFi AP started");
        break;

    case RT_WLAN_EVT_AP_STOP:
        LOG_I("WiFi AP stopped");
        break;

    case RT_WLAN_EVT_AP_ASSOCIATED:
        LOG_I("WiFi AP: Station associated");
        break;

    case RT_WLAN_EVT_AP_DISASSOCIATED:
        LOG_I("WiFi AP: Station disassociated");
        break;

    default:
        LOG_D("WiFi event: %d", event);
        break;
    }
}

/**
 * @brief 初始化WiFi管理模块
 */
int wifi_manager_init(void)
{
    /* 初始化事件 */
    rt_event_init(&wifi_event, "wifi_evt", RT_IPC_FLAG_FIFO);

    /* 注册WiFi事件回调 */
    rt_wlan_register_event_handler(RT_WLAN_EVT_READY, wifi_event_handler, RT_NULL);
    rt_wlan_register_event_handler(RT_WLAN_EVT_STA_CONNECTED, wifi_event_handler, RT_NULL);
    rt_wlan_register_event_handler(RT_WLAN_EVT_STA_DISCONNECTED, wifi_event_handler, RT_NULL);
    rt_wlan_register_event_handler(RT_WLAN_EVT_STA_CONNECTED_FAIL, wifi_event_handler, RT_NULL);
    rt_wlan_register_event_handler(RT_WLAN_EVT_AP_START, wifi_event_handler, RT_NULL);
    rt_wlan_register_event_handler(RT_WLAN_EVT_AP_STOP, wifi_event_handler, RT_NULL);
    rt_wlan_register_event_handler(RT_WLAN_EVT_AP_ASSOCIATED, wifi_event_handler, RT_NULL);
    rt_wlan_register_event_handler(RT_WLAN_EVT_AP_DISASSOCIATED, wifi_event_handler, RT_NULL);

    LOG_I("WiFi manager initialized");
    return 0;
}

/**
 * @brief 连接到指定的WiFi AP
 */
int wifi_connect(const char *ssid, const char *password)
{
    int result;

    if (ssid == RT_NULL)
    {
        LOG_E("SSID is NULL");
        return -1;
    }

    LOG_I("Connecting to WiFi: %s", ssid);
    g_wifi_status = WIFI_STATUS_CONNECTING;

    /* 连接WiFi */
    result = rt_wlan_connect(ssid, password);
    if (result != RT_EOK)
    {
        LOG_E("WiFi connect failed: %d", result);
        g_wifi_status = WIFI_STATUS_CONNECT_FAILED;
        return -1;
    }

    return 0;
}

/**
 * @brief 断开WiFi连接
 */
int wifi_disconnect(void)
{
    int result;

    LOG_I("Disconnecting WiFi");

    result = rt_wlan_disconnect();
    if (result != RT_EOK)
    {
        LOG_E("WiFi disconnect failed: %d", result);
        return -1;
    }

    g_wifi_status = WIFI_STATUS_DISCONNECTED;
    return 0;
}

/**
 * @brief 获取WiFi连接状态
 */
wifi_status_t wifi_get_status(void)
{
    return g_wifi_status;
}

/**
 * @brief 获取本地IP地址
 */
int wifi_get_ip(char *ip_buf, int buf_len)
{
    struct netdev *netdev_obj;
    char *ip_str;

    if (ip_buf == RT_NULL || buf_len <= 0)
    {
        return -1;
    }

    /* 获取WLAN网络设备 */
    netdev_obj = netdev_get_by_name("wlan0");
    if (netdev_obj == RT_NULL)
    {
        return -1;
    }

    /* 获取IP地址字符串 */
    ip_str = inet_ntoa(netdev_obj->ip_addr);
    if (ip_str == RT_NULL)
    {
        return -1;
    }

    /* 检查IP是否为0.0.0.0 */
    if (strcmp(ip_str, "0.0.0.0") == 0)
    {
        return -1;
    }

    /* 复制IP地址 */
    strncpy(ip_buf, ip_str, buf_len - 1);
    ip_buf[buf_len - 1] = '\0';

    return 0;
}

/**
 * @brief 等待WiFi连接成功
 */
int wifi_wait_ready(int timeout_ms)
{
    rt_uint32_t recv_set = 0;
    rt_uint32_t wait_set = (1 << 0);
    rt_err_t result;

    /* 如果已经连接,直接返回 */
    if (g_wifi_status == WIFI_STATUS_CONNECTED)
    {
        return 0;
    }

    /* 等待WiFi READY事件 */
    result = rt_event_recv(&wifi_event,
                          wait_set,
                          RT_EVENT_FLAG_OR | RT_EVENT_FLAG_CLEAR,
                          rt_tick_from_millisecond(timeout_ms),
                          &recv_set);

    if (result == RT_EOK)
    {
        LOG_I("WiFi ready, IP obtained");
        return 0;
    }
    else
    {
        LOG_W("Wait WiFi ready timeout");
        return -1;
    }
}
