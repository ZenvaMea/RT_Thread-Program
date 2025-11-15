/*
 * Copyright (c) 2006-2025, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2025-01-14     Cc           WiFi管理模块实现
 */

#include "wifi_manager.h"
#include "hmi_display.h"
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

#ifdef RT_USING_FINSH
#include <finsh.h>
#endif

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

        /* 更新HMI显示 */
        {
            struct rt_wlan_info info;
            char ip_buf[16] = {0};
            int rssi = 0;

            /* 获取WiFi信息 */
            if (rt_wlan_get_info(&info) == RT_EOK)
            {
                rssi = info.rssi;
            }

            /* 获取IP地址 */
            wifi_get_ip(ip_buf, sizeof(ip_buf));

            /* 更新HMI WiFi状态显示 */
            hmi_update_wifi_status((const char *)info.ssid.val, ip_buf, rssi);
            hmi_set_text("t_msg", "WiFi Connected!");
        }
        break;

    case RT_WLAN_EVT_STA_CONNECTED:
        LOG_I("WiFi STA connected");
        break;

    case RT_WLAN_EVT_STA_DISCONNECTED:
        LOG_W("WiFi STA disconnected");
        g_wifi_status = WIFI_STATUS_DISCONNECTED;

        /* 更新HMI显示 */
        hmi_update_wifi_status(NULL, NULL, 0);
        hmi_set_text("t_msg", "WiFi Disconnected");
        break;

    case RT_WLAN_EVT_STA_CONNECTED_FAIL:
        LOG_E("WiFi STA connect failed");
        g_wifi_status = WIFI_STATUS_CONNECT_FAILED;

        /* 更新HMI显示 */
        hmi_update_wifi_status(NULL, NULL, 0);
        hmi_set_text("t_msg", "WiFi Connect Failed");
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

/**
 * @brief MSH命令：连接ESP32 WiFi AP
 */
static int connect_esp32(void)
{
    const char *ssid = "ESP32_DEV";
    const char *password = "12345678";

    LOG_I("Connecting to ESP32 AP...");
    LOG_I("SSID: %s", ssid);

    /* 连接WiFi */
    if (wifi_connect(ssid, password) != 0)
    {
        LOG_E("Failed to start WiFi connection");
        return -1;
    }

    /* 等待连接成功并获取IP */
    LOG_I("Waiting for WiFi ready (30s timeout)...");
    if (wifi_wait_ready(30000) != 0)
    {
        LOG_E("WiFi connection timeout");
        return -1;
    }

    /* 获取IP地址 */
    char ip_buf[16];
    if (wifi_get_ip(ip_buf, sizeof(ip_buf)) == 0)
    {
        LOG_I("WiFi connected successfully!");
        LOG_I("IP Address: %s", ip_buf);

        /* 更新HMI显示 */
        hmi_set_text("t_msg", "WiFi Connected!");
    }
    else
    {
        LOG_W("Connected but no IP address");
    }

    return 0;
}
MSH_CMD_EXPORT(connect_esp32, Connect to ESP32 WiFi AP);

/**
 * @brief MSH命令：连接指定WiFi
 */
static int wifi_join(int argc, char **argv)
{
    char ip_buf[16];

    if (argc < 2)
    {
        rt_kprintf("Usage: wifi_join <ssid> [password]\n");
        rt_kprintf("Example: wifi_join MyWiFi 12345678\n");
        return -1;
    }

    const char *ssid = argv[1];
    const char *password = (argc >= 3) ? argv[2] : NULL;

    rt_kprintf("Connecting to WiFi: %s\n", ssid);

    /* 连接WiFi */
    if (wifi_connect(ssid, password) != 0)
    {
        rt_kprintf("Failed to start WiFi connection\n");
        return -1;
    }

    /* 等待连接成功并获取IP */
    rt_kprintf("Waiting for WiFi ready (30s timeout)...\n");
    if (wifi_wait_ready(30000) != 0)
    {
        rt_kprintf("WiFi connection timeout\n");
        return -1;
    }

    /* 获取IP地址 */
    if (wifi_get_ip(ip_buf, sizeof(ip_buf)) == 0)
    {
        rt_kprintf("WiFi connected successfully!\n");
        rt_kprintf("IP Address: %s\n", ip_buf);

        /* 更新HMI显示 */
        hmi_update_wifi_status(ssid, ip_buf, -50);
        hmi_set_text("t_msg", "WiFi Connected!");
    }

    return 0;
}
MSH_CMD_EXPORT(wifi_join, Connect to WiFi network);

/**
 * @brief MSH命令：断开WiFi连接
 */
static int wifi_leave(void)
{
    rt_kprintf("Disconnecting WiFi...\n");

    if (wifi_disconnect() == 0)
    {
        rt_kprintf("WiFi disconnected\n");

        /* 更新HMI显示 */
        hmi_update_wifi_status(NULL, NULL, 0);
        hmi_set_text("t_msg", "WiFi Disconnected");
    }
    else
    {
        rt_kprintf("Failed to disconnect WiFi\n");
        return -1;
    }

    return 0;
}
MSH_CMD_EXPORT(wifi_leave, Disconnect WiFi);

/**
 * @brief MSH命令：查看WiFi状态
 */
static int wifi_info(void)
{
    wifi_status_t status = wifi_get_status();
    char ip_buf[16] = {0};
    struct rt_wlan_info info;

    rt_kprintf("========== WiFi Status ==========\n");

    /* 状态 */
    rt_kprintf("Status: ");
    switch (status)
    {
        case WIFI_STATUS_DISCONNECTED:
            rt_kprintf("Disconnected\n");
            break;
        case WIFI_STATUS_CONNECTING:
            rt_kprintf("Connecting...\n");
            break;
        case WIFI_STATUS_CONNECTED:
            rt_kprintf("Connected\n");
            break;
        case WIFI_STATUS_CONNECT_FAILED:
            rt_kprintf("Connect Failed\n");
            break;
        default:
            rt_kprintf("Unknown\n");
            break;
    }

    /* 如果已连接,显示详细信息 */
    if (status == WIFI_STATUS_CONNECTED)
    {
        /* 获取WiFi信息 */
        if (rt_wlan_get_info(&info) == RT_EOK)
        {
            rt_kprintf("SSID: %s\n", info.ssid.val);
            rt_kprintf("BSSID: %02X:%02X:%02X:%02X:%02X:%02X\n",
                      info.bssid[0], info.bssid[1], info.bssid[2],
                      info.bssid[3], info.bssid[4], info.bssid[5]);
            rt_kprintf("RSSI: %d dBm\n", info.rssi);
            rt_kprintf("Channel: %d\n", info.channel);
        }

        /* 获取IP地址 */
        if (wifi_get_ip(ip_buf, sizeof(ip_buf)) == 0)
        {
            rt_kprintf("IP Address: %s\n", ip_buf);
        }
        else
        {
            rt_kprintf("IP Address: Not assigned\n");
        }
    }

    rt_kprintf("=================================\n");

    return 0;
}
MSH_CMD_EXPORT(wifi_info, Show WiFi connection information);

