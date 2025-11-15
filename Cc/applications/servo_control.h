/*
 * Copyright (c) 2006-2025, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2025-01-14     Cc           舵机控制接口
 */

#ifndef __SERVO_CONTROL_H__
#define __SERVO_CONTROL_H__

#include <rtthread.h>

/* ESP32服务器配置 */
#define ESP32_SERVER_IP     "192.168.4.1"
#define ESP32_SERVER_PORT   80

/* 舵机控制命令定义(基于ESP32的CONNECT.h) */
typedef enum {
    SERVO_CMD_MOVE_MIDDLE = 1,      /* 移动到中间位置 */
    SERVO_CMD_STOP = 2,              /* 停止舵机 */
    SERVO_CMD_TORQUE_OFF = 3,        /* 关闭扭矩 */
    SERVO_CMD_TORQUE_ON = 4,         /* 打开扭矩 */
    SERVO_CMD_MOVE_MAX = 5,          /* 移动到最大位置 */
    SERVO_CMD_MOVE_MIN = 6,          /* 移动到最小位置 */
    SERVO_CMD_SPEED_UP = 7,          /* 速度+100 */
    SERVO_CMD_SPEED_DOWN = 8,        /* 速度-100 */
    SERVO_CMD_SET_MIDDLE = 11,       /* 设置中点 */
    SERVO_CMD_MODE_SERVO = 12,       /* 设置为舵机模式 */
    SERVO_CMD_MODE_MOTOR = 13,       /* 设置为电机模式 */
} servo_cmd_t;

/**
 * @brief 初始化舵机控制模块
 * @param server_ip ESP32服务器IP地址(如果为NULL则使用默认IP)
 * @return 0: 成功, -1: 失败
 */
int servo_control_init(const char *server_ip);

/**
 * @brief 发送舵机控制命令
 * @param cmd 舵机命令
 * @return 0: 成功, -1: 失败
 */
int servo_send_command(servo_cmd_t cmd);

/**
 * @brief 切换活动舵机
 * @param direction 方向: 1-下一个, -1-上一个
 * @return 0: 成功, -1: 失败
 */
int servo_select_next(int direction);

/**
 * @brief 读取舵机状态
 * @param status_buf 存储状态信息的缓冲区
 * @param buf_len 缓冲区长度
 * @return 0: 成功, -1: 失败
 */
int servo_read_status(char *status_buf, int buf_len);

/**
 * @brief 读取舵机ID列表
 * @param id_buf 存储ID列表的缓冲区
 * @param buf_len 缓冲区长度
 * @return 0: 成功, -1: 失败
 */
int servo_read_id_list(char *id_buf, int buf_len);

/**
 * @brief 控制舵机移动到中间位置
 * @return 0: 成功, -1: 失败
 */
int servo_move_middle(void);

/**
 * @brief 控制舵机停止
 * @return 0: 成功, -1: 失败
 */
int servo_stop(void);

/**
 * @brief 控制舵机移动到最大位置
 * @return 0: 成功, -1: 失败
 */
int servo_move_max(void);

/**
 * @brief 控制舵机移动到最小位置
 * @return 0: 成功, -1: 失败
 */
int servo_move_min(void);

/**
 * @brief 控制舵机扭矩开关
 * @param enable 1-打开扭矩, 0-关闭扭矩
 * @return 0: 成功, -1: 失败
 */
int servo_enable_torque(int enable);

/**
 * @brief 设置舵机速度
 * @param speed_up 1-加速, 0-减速
 * @return 0: 成功, -1: 失败
 */
int servo_set_speed(int speed_up);

/**
 * @brief 设置舵机模式
 * @param motor_mode 1-电机模式, 0-舵机模式
 * @return 0: 成功, -1: 失败
 */
int servo_set_mode(int motor_mode);

#endif /* __SERVO_CONTROL_H__ */
