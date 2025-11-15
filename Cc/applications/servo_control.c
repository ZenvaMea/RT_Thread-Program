/*
 * Copyright (c) 2006-2025, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2025-01-14     Cc           舵机控制接口实现
 */

#include "servo_control.h"
#include "servo_http_client.h"
#include <rtthread.h>
#include <string.h>

#define DBG_TAG "servo.ctrl"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

static char g_server_ip[16] = ESP32_SERVER_IP;
static struct rt_mutex servo_lock;

/**
 * @brief 初始化舵机控制模块
 */
int servo_control_init(const char *server_ip)
{
    /* 初始化互斥锁 */
    rt_mutex_init(&servo_lock, "servo_lock", RT_IPC_FLAG_FIFO);

    /* 设置服务器IP */
    if (server_ip != RT_NULL)
    {
        strncpy(g_server_ip, server_ip, sizeof(g_server_ip) - 1);
    }

    LOG_I("Servo control initialized, server: %s", g_server_ip);
    return 0;
}

/**
 * @brief 构建控制URL
 */
static int build_command_url(char *url, int url_len, int cmd_type, int cmd_id, int cmd_a, int cmd_b)
{
    rt_snprintf(url, url_len,
                "http://%s/cmd?t=%d&i=%d&a=%d&b=%d",
                g_server_ip, cmd_type, cmd_id, cmd_a, cmd_b);
    return 0;
}

/**
 * @brief 发送舵机控制命令
 */
int servo_send_command(servo_cmd_t cmd)
{
    char url[128];
    int ret;

    /* 构建命令URL */
    build_command_url(url, sizeof(url), 1, (int)cmd, 0, 0);

    /* 获取互斥锁 */
    rt_mutex_take(&servo_lock, RT_WAITING_FOREVER);

    /* 发送HTTP请求 */
    LOG_D("Sending command: %s", url);
    ret = http_get_simple(url);

    /* 释放互斥锁 */
    rt_mutex_release(&servo_lock);

    if (ret == 0)
    {
        LOG_I("Command sent successfully");
        return 0;
    }
    else
    {
        LOG_E("Command send failed");
        return -1;
    }
}

/**
 * @brief 切换活动舵机
 */
int servo_select_next(int direction)
{
    char url[128];
    int ret;

    /* 构建URL: t=0表示切换舵机 */
    build_command_url(url, sizeof(url), 0, direction, 0, 0);

    /* 获取互斥锁 */
    rt_mutex_take(&servo_lock, RT_WAITING_FOREVER);

    /* 发送HTTP请求 */
    LOG_D("Selecting servo: %s", url);
    ret = http_get_simple(url);

    /* 释放互斥锁 */
    rt_mutex_release(&servo_lock);

    if (ret == 0)
    {
        LOG_I("Servo selected");
        return 0;
    }
    else
    {
        LOG_E("Select servo failed");
        return -1;
    }
}

/**
 * @brief 读取舵机状态
 */
int servo_read_status(char *status_buf, int buf_len)
{
    char url[128];
    http_response_t response = {0};
    int ret = -1;

    if (status_buf == RT_NULL || buf_len <= 0)
    {
        return -1;
    }

    /* 构建URL */
    rt_snprintf(url, sizeof(url), "http://%s/readSTS", g_server_ip);

    /* 获取互斥锁 */
    rt_mutex_take(&servo_lock, RT_WAITING_FOREVER);

    /* 发送HTTP请求 */
    if (http_get(url, &response, 3000) == 0)
    {
        if (response.body && response.body_len > 0)
        {
            int copy_len = response.body_len < (buf_len - 1) ? response.body_len : (buf_len - 1);
            strncpy(status_buf, response.body, copy_len);
            status_buf[copy_len] = '\0';
            ret = 0;
        }
        http_response_free(&response);
    }

    /* 释放互斥锁 */
    rt_mutex_release(&servo_lock);

    return ret;
}

/**
 * @brief 读取舵机ID列表
 */
int servo_read_id_list(char *id_buf, int buf_len)
{
    char url[128];
    http_response_t response = {0};
    int ret = -1;

    if (id_buf == RT_NULL || buf_len <= 0)
    {
        return -1;
    }

    /* 构建URL */
    rt_snprintf(url, sizeof(url), "http://%s/readID", g_server_ip);

    /* 获取互斥锁 */
    rt_mutex_take(&servo_lock, RT_WAITING_FOREVER);

    /* 发送HTTP请求 */
    if (http_get(url, &response, 3000) == 0)
    {
        if (response.body && response.body_len > 0)
        {
            int copy_len = response.body_len < (buf_len - 1) ? response.body_len : (buf_len - 1);
            strncpy(id_buf, response.body, copy_len);
            id_buf[copy_len] = '\0';
            ret = 0;
        }
        http_response_free(&response);
    }

    /* 释放互斥锁 */
    rt_mutex_release(&servo_lock);

    return ret;
}

/**
 * @brief 控制舵机移动到中间位置
 */
int servo_move_middle(void)
{
    return servo_send_command(SERVO_CMD_MOVE_MIDDLE);
}

/**
 * @brief 控制舵机停止
 */
int servo_stop(void)
{
    return servo_send_command(SERVO_CMD_STOP);
}

/**
 * @brief 控制舵机移动到最大位置
 */
int servo_move_max(void)
{
    return servo_send_command(SERVO_CMD_MOVE_MAX);
}

/**
 * @brief 控制舵机移动到最小位置
 */
int servo_move_min(void)
{
    return servo_send_command(SERVO_CMD_MOVE_MIN);
}

/**
 * @brief 控制舵机扭矩开关
 */
int servo_enable_torque(int enable)
{
    if (enable)
    {
        return servo_send_command(SERVO_CMD_TORQUE_ON);
    }
    else
    {
        return servo_send_command(SERVO_CMD_TORQUE_OFF);
    }
}

/**
 * @brief 设置舵机速度
 */
int servo_set_speed(int speed_up)
{
    if (speed_up)
    {
        return servo_send_command(SERVO_CMD_SPEED_UP);
    }
    else
    {
        return servo_send_command(SERVO_CMD_SPEED_DOWN);
    }
}

/**
 * @brief 设置舵机模式
 */
int servo_set_mode(int motor_mode)
{
    if (motor_mode)
    {
        return servo_send_command(SERVO_CMD_MODE_MOTOR);
    }
    else
    {
        return servo_send_command(SERVO_CMD_MODE_SERVO);
    }
}
