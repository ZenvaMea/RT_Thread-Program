/*
 * Copyright (c) 2006-2020, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-09-02     RT-Thread    first version
 * 2025-01-14     Claude       Add WiFi servo control
 */

#include <rtthread.h>
#include <rtdevice.h>
#include "drv_common.h"
#include "wifi_manager.h"
#include "servo_http_client.h"
#include "servo_control.h"
#include "servo_advanced.h"

#define DBG_TAG "main"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>


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

    LOG_I("==============================================");
    LOG_I("ART-PI WiFi Servo Control System");
    LOG_I("==============================================");
    LOG_I("Quick start:");
    LOG_I("  1. Use 'connect_esp32' to connect WiFi");
    LOG_I("  2. Use 'serv' command to control servos");
    LOG_I("     Example: serv move 0 0 2  (servo 0, middle, med speed)");
    LOG_I("             serv all_mid 3    (all to middle, fast)");
    LOG_I("             serv wave 3 2     (wave 3 times, med speed)");
    LOG_I("             servo_test        (run full test)");
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

