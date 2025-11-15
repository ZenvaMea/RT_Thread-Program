/*
 * Copyright (c) 2006-2020, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-09-02     RT-Thread    first version
 * 2025-01-14     Cc           Add WiFi servo control
 */

#include <rtthread.h>
#include <rtdevice.h>
#include "drv_common.h"
#include "wifi_manager.h"
#include "servo_http_client.h"
#include "servo_control.h"
#include "servo_advanced.h"
#include "hmi_display.h"

#define DBG_TAG "main"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

/* System info update thread */
static void sys_info_thread_entry(void *parameter)
{
    rt_uint32_t start_tick = rt_tick_get();

    rt_thread_mdelay(3000);  /* Wait for system stabilization */

    while (1)
    {
        /* Calculate runtime */
        rt_uint32_t runtime_sec = (rt_tick_get() - start_tick) / RT_TICK_PER_SECOND;

        /* Update runtime display */
        hmi_update_runtime(runtime_sec);

        /* Update CPU usage (simple estimation) */
        /* TODO: Implement actual CPU usage calculation */
        hmi_update_cpu_usage(15);  /* Placeholder value */

        /* Update memory usage */
        rt_uint32_t total, used, max_used;
        rt_memory_info(&total, &used, &max_used);
        hmi_update_memory_info(used / 1024, total / 1024);

        /* Update every 1 second */
        rt_thread_mdelay(1000);
    }
}


int main(void)
{
    rt_uint32_t count = 1;


    /* 初始化WiFi管理模块 */
    wifi_manager_init();
    LOG_I("WiFi manager initialized");

    /* 初始化HTTP客户端 */
    http_client_init();
    LOG_I("HTTP client initialized");

    /* 初始化舵机控制模块 */
    servo_control_init(NULL);
    LOG_I("Servo control initialized");

    /* 初始化高级舵机控制模块 */
    servo_advanced_init();
    LOG_I("Servo advanced control initialized");

    /* 初始化串口屏显示模块 */
    if (hmi_init() == RT_EOK)
    {
        LOG_I("HMI display initialized successfully");

        /* 启动接收线程 */
        hmi_start_thread();

        /* 显示欢迎信息 */
        hmi_set_text("t_msg", "System Ready!");
        hmi_set_text("t_wifi", "WiFi: Disconnected");
        hmi_update_servo_pos(1, 0);
        hmi_update_servo_pos(2, 0);
        hmi_update_servo_pos(3, 0);
        hmi_update_servo_pos(4, 0);

        /* 创建系统信息更新线程 */
        rt_thread_t sys_thread = rt_thread_create("sys_info",
                                                   sys_info_thread_entry,
                                                   RT_NULL,
                                                   2048,
                                                   20,
                                                   20);
        if (sys_thread != RT_NULL)
        {
            rt_thread_startup(sys_thread);
            LOG_I("System info update thread started");
        }
    }
    else
    {
        LOG_W("HMI display initialization failed");
    }

    LOG_I("==============================================");
    LOG_I("ART-PI WiFi Servo Control System with HMI");
    LOG_I("==============================================");
    LOG_I("Quick start:");
    LOG_I("  1. Use 'connect_esp32' to connect WiFi");
    LOG_I("  2. Use 'serv' command to control servos");
    LOG_I("     Example: serv move 0 0 2  (servo 0, middle, med speed)");
    LOG_I("             serv all_mid 3    (all to middle, fast)");
    LOG_I("             serv wave 3 2     (wave 3 times, med speed)");
    LOG_I("             servo_test        (run full test)");
    LOG_I("  3. Use 'hmi_test' to test HMI display");
    LOG_I("     Example: hmi_test text t0 Hello");
    LOG_I("             hmi_test wifi");
    LOG_I("             hmi_test servo 1 2048");
    LOG_I("==============================================");


    return RT_EOK;
}

#include "stm32h7rsxx.h"
static int vtor_config(void)
{
    /* Vector Table Relocation in Internal XSPI2_BASE */
    SCB->VTOR = XSPI2_BASE;
    return 0;
}
INIT_BOARD_EXPORT(vtor_config);

