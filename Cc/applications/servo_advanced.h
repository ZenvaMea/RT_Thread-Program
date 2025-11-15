/*
 * Copyright (c) 2006-2025, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2025-01-14     Claude       舵机高级控制接口
 */

#ifndef __SERVO_ADVANCED_H__
#define __SERVO_ADVANCED_H__

#include <rtthread.h>

/* 舵机位置定义 */
#define SERVO_POS_MIDDLE    0   /* 中间位置 */
#define SERVO_POS_MAX       1   /* 最大位置 */
#define SERVO_POS_MIN       2   /* 最小位置 */

/* 速度级别定义 */
#define SERVO_SPEED_SLOW    1   /* 慢速 */
#define SERVO_SPEED_MEDIUM  2   /* 中速 */
#define SERVO_SPEED_FAST    3   /* 快速 */
#define SERVO_SPEED_MAX     4   /* 最快 */

/* 舵机动作结构 */
typedef struct {
    int servo_id;           /* 舵机ID (0-3表示第0-3个舵机) */
    int position;           /* 目标位置 (SERVO_POS_MIDDLE/MAX/MIN) */
    int speed;              /* 运动速度级别 (SERVO_SPEED_SLOW/MEDIUM/FAST/MAX) */
    int delay_ms;           /* 动作后延时(毫秒) */
} servo_action_t;

/* 舵机组控制结构 */
typedef struct {
    int servo_ids[4];       /* 舵机ID列表 */
    int positions[4];       /* 对应位置 */
    int count;              /* 舵机数量 */
    int speed;              /* 统一速度级别 */
} servo_group_t;

/**
 * @brief 初始化高级舵机控制模块
 * @return 0: 成功, -1: 失败
 */
int servo_advanced_init(void);

/**
 * @brief 按ID控制指定舵机移动到位置
 * @param servo_id 舵机ID (0-3)
 * @param position 目标位置 (SERVO_POS_MIDDLE/MAX/MIN)
 * @return 0: 成功, -1: 失败
 */
int servo_move_by_id(int servo_id, int position);

/**
 * @brief 按ID控制指定舵机移动到位置（带速度）
 * @param servo_id 舵机ID (0-3)
 * @param position 目标位置
 * @param speed 速度级别 (SERVO_SPEED_SLOW/MEDIUM/FAST/MAX)
 * @return 0: 成功, -1: 失败
 */
int servo_move_by_id_speed(int servo_id, int position, int speed);

/**
 * @brief 同时控制所有舵机移动到中间位置
 * @return 0: 成功, -1: 失败
 */
int servo_all_middle(void);

/**
 * @brief 同时控制所有舵机移动到中间位置（带速度）
 * @param speed 速度级别
 * @return 0: 成功, -1: 失败
 */
int servo_all_middle_speed(int speed);

/**
 * @brief 同时控制所有舵机停止
 * @return 0: 成功, -1: 失败
 */
int servo_all_stop(void);

/**
 * @brief 同时控制所有舵机扭矩开关
 * @param enable 1-打开, 0-关闭
 * @return 0: 成功, -1: 失败
 */
int servo_all_torque(int enable);

/**
 * @brief 设置所有舵机速度
 * @param speed 速度级别
 * @return 0: 成功, -1: 失败
 */
int servo_all_set_speed(int speed);

/**
 * @brief 控制多个舵机移动到指定位置
 * @param servo_ids 舵机ID数组
 * @param positions 位置数组
 * @param count 舵机数量
 * @return 0: 成功, -1: 失败
 */
int servo_multi_move(int *servo_ids, int *positions, int count);

/**
 * @brief 控制多个舵机移动到指定位置（带速度）
 * @param servo_ids 舵机ID数组
 * @param positions 位置数组
 * @param speeds 速度数组
 * @param count 舵机数量
 * @return 0: 成功, -1: 失败
 */
int servo_multi_move_speed(int *servo_ids, int *positions, int *speeds, int count);

/**
 * @brief 执行舵机动作序列
 * @param actions 动作数组
 * @param count 动作数量
 * @return 0: 成功, -1: 失败
 */
int servo_execute_sequence(servo_action_t *actions, int count);

/**
 * @brief 控制舵机组（同时控制多个舵机到不同位置）
 * @param group 舵机组配置
 * @return 0: 成功, -1: 失败
 */
int servo_group_control(servo_group_t *group);

/**
 * @brief 创建预设动作：所有舵机回中位
 * @return 0: 成功, -1: 失败
 */
int servo_preset_home(void);

/**
 * @brief 创建预设动作：舵机波浪动作
 * @param cycles 波浪循环次数
 * @param speed 速度级别
 * @return 0: 成功, -1: 失败
 */
int servo_preset_wave(int cycles, int speed);

/**
 * @brief 创建预设动作：舵机依次动作
 * @param speed 速度级别
 * @return 0: 成功, -1: 失败
 */
int servo_preset_sequence(int speed);

/**
 * @brief 设置当前活动舵机ID（用于优化）
 * @param servo_id 舵机ID (0-3)
 * @return 0: 成功, -1: 失败
 */
int servo_set_active_id(int servo_id);

/**
 * @brief 获取当前活动舵机ID
 * @return 当前活动舵机ID
 */
int servo_get_active_id(void);

#endif /* __SERVO_ADVANCED_H__ */
