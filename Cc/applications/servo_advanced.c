/*
 * Copyright (c) 2006-2025, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2025-01-14       Claude       舵机高级控制接口实现
 */

#include "servo_advanced.h"
#include "servo_control.h"
#include <rtthread.h>

#define DBG_TAG "servo.adv"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

#define SERVO_COUNT 4           /* 舵机总数 */

static int g_current_servo_id = 0;      /* 当前选中的舵机ID */
static struct rt_mutex advanced_lock;

/**
 * @brief 初始化高级舵机控制模块
 */
int servo_advanced_init(void)
{
    /* 初始化互斥锁 */
    rt_mutex_init(&advanced_lock, "adv_lock", RT_IPC_FLAG_FIFO);

    g_current_servo_id = 0;

    LOG_I("Servo advanced control initialized");
    return 0;
}

/**
 * @brief 切换到指定舵机
 */
static int switch_to_servo(int target_id)
{
    int steps;
    int i;

    if (target_id < 0 || target_id >= SERVO_COUNT)
    {
        LOG_E("Invalid servo ID: %d", target_id);
        return -1;
    }

    /* 如果已经是目标舵机，直接返回 */
    if (g_current_servo_id == target_id)
    {
        return 0;
    }

    /* 计算需要切换的步数 */
    steps = target_id - g_current_servo_id;

    LOG_D("Switching from servo %d to %d (steps: %d)",
          g_current_servo_id, target_id, steps);

    /* 执行切换 */
    if (steps > 0)
    {
        /* 向前切换 */
        for (i = 0; i < steps; i++)
        {
            if (servo_select_next(1) != 0)
            {
                LOG_E("Failed to select next servo");
                return -1;
            }
            rt_thread_mdelay(50); /* 短暂延时确保切换成功 */
        }
    }
    else if (steps < 0)
    {
        /* 向后切换 */
        for (i = 0; i < -steps; i++)
        {
            if (servo_select_next(-1) != 0)
            {
                LOG_E("Failed to select previous servo");
                return -1;
            }
            rt_thread_mdelay(50);
        }
    }

    g_current_servo_id = target_id;
    return 0;
}

/**
 * @brief 根据速度级别设置舵机速度
 */
static int set_servo_speed_level(int speed_level)
{
    int i;
    int adjust_times;

    if (speed_level <= 0)
    {
        return 0; /* 速度为0表示使用当前速度 */
    }

    /* 根据速度级别调整速度 */
    switch (speed_level)
    {
    case SERVO_SPEED_SLOW:
        /* 降速：执行多次speed down */
        adjust_times = 10;
        for (i = 0; i < adjust_times; i++)
        {
            servo_set_speed(0); /* 降速 */
            rt_thread_mdelay(10);
        }
        break;

    case SERVO_SPEED_MEDIUM:
        /* 中速：执行几次speed down */
        adjust_times = 5;
        for (i = 0; i < adjust_times; i++)
        {
            servo_set_speed(0);
            rt_thread_mdelay(10);
        }
        break;

    case SERVO_SPEED_FAST:
        /* 快速：执行几次speed up */
        adjust_times = 5;
        for (i = 0; i < adjust_times; i++)
        {
            servo_set_speed(1); /* 加速 */
            rt_thread_mdelay(10);
        }
        break;

    case SERVO_SPEED_MAX:
        /* 最快：执行多次speed up */
        adjust_times = 10;
        for (i = 0; i < adjust_times; i++)
        {
            servo_set_speed(1);
            rt_thread_mdelay(10);
        }
        break;

    default:
        LOG_W("Unknown speed level: %d", speed_level);
        return -1;
    }

    return 0;
}

/**
 * @brief 根据位置代码执行舵机动作
 */
static int execute_position_cmd(int position)
{
    switch (position)
    {
    case SERVO_POS_MIDDLE: /* 中间位置 */
        return servo_move_middle();
    case SERVO_POS_MAX: /* 最大位置 */
        return servo_move_max();
    case SERVO_POS_MIN: /* 最小位置 */
        return servo_move_min();
    default:
        LOG_E("Invalid position: %d", position);
        return -1;
    }
}

/**
 * @brief 按ID控制指定舵机移动到位置
 */
int servo_move_by_id(int servo_id, int position)
{
    return servo_move_by_id_speed(servo_id, position, 0);
}

/**
 * @brief 按ID控制指定舵机移动到位置（带速度）
 */
int servo_move_by_id_speed(int servo_id, int position, int speed)
{
    int ret;

    /* 获取互斥锁 */
    rt_mutex_take(&advanced_lock, RT_WAITING_FOREVER);

    /* 切换到目标舵机 */
    if (switch_to_servo(servo_id) != 0)
    {
        rt_mutex_release(&advanced_lock);
        return -1;
    }

    /* 设置速度 */
    if (speed > 0)
    {
        set_servo_speed_level(speed);
    }

    /* 执行动作 */
    ret = execute_position_cmd(position);

    /* 释放互斥锁 */
    rt_mutex_release(&advanced_lock);

    if (ret == 0)
    {
        LOG_I("Servo %d moved to position %d (speed: %d)", servo_id, position, speed);
    }

    return ret;
}

/**
 * @brief 同时控制所有舵机移动到中间位置
 */
int servo_all_middle(void)
{
    return servo_all_middle_speed(0);
}

/**
 * @brief 同时控制所有舵机移动到中间位置（带速度）
 */
int servo_all_middle_speed(int speed)
{
    int i;
    int ret = 0;

    LOG_I("Moving all servos to middle position (speed: %d)", speed);

    /* 获取互斥锁 */
    rt_mutex_take(&advanced_lock, RT_WAITING_FOREVER);

    for (i = 0; i < SERVO_COUNT; i++)
    {
        /* 切换到舵机 */
        if (switch_to_servo(i) != 0)
        {
            ret = -1;
            break;
        }

        /* 设置速度 */
        if (speed > 0)
        {
            set_servo_speed_level(speed);
        }

        /* 移动到中间位置 */
        if (servo_move_middle() != 0)
        {
            LOG_E("Failed to move servo %d", i);
            ret = -1;
            break;
        }

        rt_thread_mdelay(100); /* 给舵机一点时间开始运动 */
    }

    /* 释放互斥锁 */
    rt_mutex_release(&advanced_lock);

    return ret;
}

/**
 * @brief 同时控制所有舵机停止
 */
int servo_all_stop(void)
{
    int i;
    int ret = 0;

    LOG_I("Stopping all servos");

    /* 获取互斥锁 */
    rt_mutex_take(&advanced_lock, RT_WAITING_FOREVER);

    for (i = 0; i < SERVO_COUNT; i++)
    {
        if (switch_to_servo(i) != 0)
        {
            ret = -1;
            break;
        }

        if (servo_stop() != 0)
        {
            LOG_E("Failed to stop servo %d", i);
            ret = -1;
            break;
        }

        rt_thread_mdelay(50);
    }

    /* 释放互斥锁 */
    rt_mutex_release(&advanced_lock);

    return ret;
}

/**
 * @brief 同时控制所有舵机扭矩开关
 */
int servo_all_torque(int enable)
{
    int i;
    int ret = 0;

    LOG_I("Setting all servos torque: %s", enable ? "ON" : "OFF");

    /* 获取互斥锁 */
    rt_mutex_take(&advanced_lock, RT_WAITING_FOREVER);

    for (i = 0; i < SERVO_COUNT; i++)
    {
        if (switch_to_servo(i) != 0)
        {
            ret = -1;
            break;
        }

        if (servo_enable_torque(enable) != 0)
        {
            LOG_E("Failed to set torque for servo %d", i);
            ret = -1;
            break;
        }

        rt_thread_mdelay(50);
    }

    /* 释放互斥锁 */
    rt_mutex_release(&advanced_lock);

    return ret;
}

/**
 * @brief 设置所有舵机速度
 */
int servo_all_set_speed(int speed)
{
    int i;
    int ret = 0;

    LOG_I("Setting all servos speed: %d", speed);

    /* 获取互斥锁 */
    rt_mutex_take(&advanced_lock, RT_WAITING_FOREVER);

    for (i = 0; i < SERVO_COUNT; i++)
    {
        if (switch_to_servo(i) != 0)
        {
            ret = -1;
            break;
        }

        if (set_servo_speed_level(speed) != 0)
        {
            LOG_E("Failed to set speed for servo %d", i);
            ret = -1;
            break;
        }

        rt_thread_mdelay(50);
    }

    /* 释放互斥锁 */
    rt_mutex_release(&advanced_lock);

    return ret;
}

/**
 * @brief 控制多个舵机移动到指定位置
 */
int servo_multi_move(int *servo_ids, int *positions, int count)
{
    return servo_multi_move_speed(servo_ids, positions, NULL, count);
}

/**
 * @brief 控制多个舵机移动到指定位置（带速度）
 */
int servo_multi_move_speed(int *servo_ids, int *positions, int *speeds, int count)
{
    int i;
    int ret = 0;

    if (servo_ids == RT_NULL || positions == RT_NULL || count <= 0)
    {
        return -1;
    }

    LOG_I("Moving %d servos", count);

    /* 获取互斥锁 */
    rt_mutex_take(&advanced_lock, RT_WAITING_FOREVER);

    for (i = 0; i < count; i++)
    {
        /* 切换到目标舵机 */
        if (switch_to_servo(servo_ids[i]) != 0)
        {
            ret = -1;
            break;
        }

        /* 设置速度（如果提供） */
        if (speeds != RT_NULL && speeds[i] > 0)
        {
            set_servo_speed_level(speeds[i]);
        }

        /* 执行动作 */
        if (execute_position_cmd(positions[i]) != 0)
        {
            LOG_E("Failed to move servo %d to position %d",
                  servo_ids[i], positions[i]);
            ret = -1;
            break;
        }

        rt_thread_mdelay(100);
    }

    /* 释放互斥锁 */
    rt_mutex_release(&advanced_lock);

    return ret;
}

/**
 * @brief 执行舵机动作序列
 */
int servo_execute_sequence(servo_action_t *actions, int count)
{
    int i;
    int ret = 0;

    if (actions == RT_NULL || count <= 0)
    {
        return -1;
    }

    LOG_I("Executing sequence with %d actions", count);

    /* 获取互斥锁 */
    rt_mutex_take(&advanced_lock, RT_WAITING_FOREVER);

    for (i = 0; i < count; i++)
    {
        LOG_D("Action %d: Servo %d, Position %d, Speed %d, Delay %d",
              i, actions[i].servo_id, actions[i].position,
              actions[i].speed, actions[i].delay_ms);

        /* 切换到目标舵机 */
        if (switch_to_servo(actions[i].servo_id) != 0)
        {
            ret = -1;
            break;
        }

        /* 设置速度 */
        if (actions[i].speed > 0)
        {
            set_servo_speed_level(actions[i].speed);
        }

        /* 执行动作 */
        if (execute_position_cmd(actions[i].position) != 0)
        {
            LOG_E("Failed to execute action %d", i);
            ret = -1;
            break;
        }

        /* 延时 */
        if (actions[i].delay_ms > 0)
        {
            rt_thread_mdelay(actions[i].delay_ms);
        }
    }

    /* 释放互斥锁 */
    rt_mutex_release(&advanced_lock);

    LOG_I("Sequence execution %s", ret == 0 ? "completed" : "failed");
    return ret;
}

/**
 * @brief 控制舵机组
 */
int servo_group_control(servo_group_t *group)
{
    int speeds[4] = {0};
    int i;

    if (group == RT_NULL || group->count <= 0 || group->count > SERVO_COUNT)
    {
        return -1;
    }

    /* 准备速度数组 */
    if (group->speed > 0)
    {
        for (i = 0; i < group->count; i++)
        {
            speeds[i] = group->speed;
        }
        return servo_multi_move_speed(group->servo_ids, group->positions, speeds, group->count);
    }
    else
    {
        return servo_multi_move(group->servo_ids, group->positions, group->count);
    }
}

/**
 * @brief 预设动作：所有舵机回中位
 */
int servo_preset_home(void)
{
    LOG_I("Executing preset: HOME");
    return servo_all_middle_speed(SERVO_SPEED_MEDIUM);
}

/**
 * @brief 预设动作：舵机波浪动作
 */
int servo_preset_wave(int cycles, int speed)
{
    int i, cycle;
    servo_action_t actions[SERVO_COUNT * 2];
    int action_count = 0;

    if (speed <= 0)
    {
        speed = SERVO_SPEED_MEDIUM;
    }

    LOG_I("Executing preset: WAVE (cycles: %d, speed: %d)", cycles, speed);

    for (cycle = 0; cycle < cycles; cycle++)
    {
        action_count = 0;

        /* 正向波浪 */
        for (i = 0; i < SERVO_COUNT; i++)
        {
            actions[action_count].servo_id = i;
            actions[action_count].position = SERVO_POS_MAX;
            actions[action_count].speed = speed;
            actions[action_count].delay_ms = 200;
            action_count++;
        }

        /* 反向波浪 */
        for (i = SERVO_COUNT - 1; i >= 0; i--)
        {
            actions[action_count].servo_id = i;
            actions[action_count].position = SERVO_POS_MIN;
            actions[action_count].speed = speed;
            actions[action_count].delay_ms = 200;
            action_count++;
        }

        /* 执行一个周期 */
        if (servo_execute_sequence(actions, action_count) != 0)
        {
            return -1;
        }
    }

    /* 回到中位 */
    return servo_all_middle_speed(speed);
}

/**
 * @brief 预设动作：舵机依次动作
 */
int servo_preset_sequence(int speed)
{
    int i;
    servo_action_t actions[SERVO_COUNT * 3];
    int action_count = 0;

    if (speed <= 0)
    {
        speed = SERVO_SPEED_MEDIUM;
    }

    LOG_I("Executing preset: SEQUENCE (speed: %d)", speed);

    /* 依次移动到最大位置 */
    for (i = 0; i < SERVO_COUNT; i++)
    {
        actions[action_count].servo_id = i;
        actions[action_count].position = SERVO_POS_MAX;
        actions[action_count].speed = speed;
        actions[action_count].delay_ms = 500;
        action_count++;
    }

    /* 依次移动到最小位置 */
    for (i = 0; i < SERVO_COUNT; i++)
    {
        actions[action_count].servo_id = i;
        actions[action_count].position = SERVO_POS_MIN;
        actions[action_count].speed = speed;
        actions[action_count].delay_ms = 500;
        action_count++;
    }

    /* 依次回到中位 */
    for (i = 0; i < SERVO_COUNT; i++)
    {
        actions[action_count].servo_id = i;
        actions[action_count].position = SERVO_POS_MIDDLE;
        actions[action_count].speed = speed;
        actions[action_count].delay_ms = 500;
        action_count++;
    }

    return servo_execute_sequence(actions, action_count);
}

/**
 * @brief 设置当前活动舵机ID
 */
int servo_set_active_id(int servo_id)
{
    if (servo_id < 0 || servo_id >= SERVO_COUNT)
    {
        return -1;
    }

    rt_mutex_take(&advanced_lock, RT_WAITING_FOREVER);
    int ret = switch_to_servo(servo_id);
    rt_mutex_release(&advanced_lock);

    return ret;
}

/**
 * @brief 获取当前活动舵机ID
 */
int servo_get_active_id(void)
{
    return g_current_servo_id;
}
