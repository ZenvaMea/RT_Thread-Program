/*
 * Copyright (c) 2006-2025, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2025-01-14     Claude       高级舵机控制MSH命令
 */

#include <rtthread.h>
#include "servo_advanced.h"

#define DBG_TAG "servo.msh_adv"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

/**
 * @brief 舵机高级控制命令
 * serv <cmd> [args]
 */
static int cmd_serv(int argc, char **argv)
{
    int ret = -1;

    if (argc < 2)
    {
        rt_kprintf("Usage: serv <command> [args]\n");
        rt_kprintf("Commands:\n");
        rt_kprintf("  move <id> <pos> [speed]   - Move servo by ID\n");
        rt_kprintf("                              id: 0-3, pos: 0(mid)/1(max)/2(min)\n");
        rt_kprintf("                              speed: 1(slow)/2(mid)/3(fast)/4(max)\n");
        rt_kprintf("  all_mid [speed]           - All servos to middle\n");
        rt_kprintf("  all_stop                  - Stop all servos\n");
        rt_kprintf("  all_ton                   - Torque on for all\n");
        rt_kprintf("  all_toff                  - Torque off for all\n");
        rt_kprintf("  all_speed <speed>         - Set all servos speed\n");
        rt_kprintf("  multi <id1,id2...> <pos1,pos2...> - Multi control\n");
        rt_kprintf("  home                      - Preset: home position\n");
        rt_kprintf("  wave <cycles> [speed]     - Preset: wave motion\n");
        rt_kprintf("  seq [speed]               - Preset: sequence motion\n");
        rt_kprintf("  active <id>               - Set active servo ID\n");
        rt_kprintf("\nExamples:\n");
        rt_kprintf("  serv move 0 0 2          - Move servo 0 to middle, medium speed\n");
        rt_kprintf("  serv move 1 1 4          - Move servo 1 to max, max speed\n");
        rt_kprintf("  serv all_mid 3           - All to middle, fast speed\n");
        rt_kprintf("  serv multi 0,1,2 0,1,2   - Servos 0,1,2 to mid,max,min\n");
        rt_kprintf("  serv wave 3 2            - Wave 3 cycles, medium speed\n");
        return -1;
    }

    /* move - 按ID移动舵机 */
    if (strcmp(argv[1], "move") == 0)
    {
        if (argc < 4)
        {
            rt_kprintf("Usage: serv move <id> <pos> [speed]\n");
            return -1;
        }

        int id = atoi(argv[2]);
        int pos = atoi(argv[3]);
        int speed = (argc >= 5) ? atoi(argv[4]) : 0;

        rt_kprintf("Moving servo %d to position %d, speed %d\n", id, pos, speed);
        ret = servo_move_by_id_speed(id, pos, speed);
    }
    /* all_mid - 所有舵机回中位 */
    else if (strcmp(argv[1], "all_mid") == 0)
    {
        int speed = (argc >= 3) ? atoi(argv[2]) : 0;
        rt_kprintf("Moving all servos to middle (speed: %d)\n", speed);
        ret = servo_all_middle_speed(speed);
    }
    /* all_stop - 所有舵机停止 */
    else if (strcmp(argv[1], "all_stop") == 0)
    {
        rt_kprintf("Stopping all servos\n");
        ret = servo_all_stop();
    }
    /* all_ton - 所有舵机扭矩打开 */
    else if (strcmp(argv[1], "all_ton") == 0)
    {
        rt_kprintf("Torque ON for all servos\n");
        ret = servo_all_torque(1);
    }
    /* all_toff - 所有舵机扭矩关闭 */
    else if (strcmp(argv[1], "all_toff") == 0)
    {
        rt_kprintf("Torque OFF for all servos\n");
        ret = servo_all_torque(0);
    }
    /* all_speed - 设置所有舵机速度 */
    else if (strcmp(argv[1], "all_speed") == 0)
    {
        if (argc < 3)
        {
            rt_kprintf("Usage: serv all_speed <speed>\n");
            return -1;
        }
        int speed = atoi(argv[2]);
        rt_kprintf("Setting all servos speed to %d\n", speed);
        ret = servo_all_set_speed(speed);
    }
    /* multi - 多舵机控制 */
    else if (strcmp(argv[1], "multi") == 0)
    {
        if (argc < 4)
        {
            rt_kprintf("Usage: serv multi <id1,id2...> <pos1,pos2...>\n");
            return -1;
        }

        int ids[4], positions[4];
        int count = 0;

        /* 解析ID列表 */
        char *id_str = argv[2];
        char *token = strtok(id_str, ",");
        while (token != NULL && count < 4)
        {
            ids[count++] = atoi(token);
            token = strtok(NULL, ",");
        }

        /* 解析位置列表 */
        int pos_count = 0;
        char *pos_str = argv[3];
        token = strtok(pos_str, ",");
        while (token != NULL && pos_count < 4)
        {
            positions[pos_count++] = atoi(token);
            token = strtok(NULL, ",");
        }

        if (count != pos_count)
        {
            rt_kprintf("Error: ID count (%d) != position count (%d)\n", count, pos_count);
            return -1;
        }

        rt_kprintf("Multi control: %d servos\n", count);
        ret = servo_multi_move(ids, positions, count);
    }
    /* home - 预设动作：回中位 */
    else if (strcmp(argv[1], "home") == 0)
    {
        rt_kprintf("Executing preset: HOME\n");
        ret = servo_preset_home();
    }
    /* wave - 预设动作：波浪 */
    else if (strcmp(argv[1], "wave") == 0)
    {
        if (argc < 3)
        {
            rt_kprintf("Usage: serv wave <cycles> [speed]\n");
            return -1;
        }
        int cycles = atoi(argv[2]);
        int speed = (argc >= 4) ? atoi(argv[3]) : SERVO_SPEED_MEDIUM;
        rt_kprintf("Executing preset: WAVE (%d cycles, speed %d)\n", cycles, speed);
        ret = servo_preset_wave(cycles, speed);
    }
    /* seq - 预设动作：依次动作 */
    else if (strcmp(argv[1], "seq") == 0)
    {
        int speed = (argc >= 3) ? atoi(argv[2]) : SERVO_SPEED_MEDIUM;
        rt_kprintf("Executing preset: SEQUENCE (speed %d)\n", speed);
        ret = servo_preset_sequence(speed);
    }
    /* active - 设置活动舵机 */
    else if (strcmp(argv[1], "active") == 0)
    {
        if (argc < 3)
        {
            rt_kprintf("Current active servo: %d\n", servo_get_active_id());
            return 0;
        }
        int id = atoi(argv[2]);
        rt_kprintf("Setting active servo to %d\n", id);
        ret = servo_set_active_id(id);
    }
    else
    {
        rt_kprintf("Unknown command: %s\n", argv[1]);
        return -1;
    }

    if (ret == 0)
    {
        rt_kprintf("Command executed successfully\n");
    }
    else
    {
        rt_kprintf("Command execution failed\n");
    }

    return ret;
}
MSH_CMD_EXPORT_ALIAS(cmd_serv, serv, Advanced servo control);

/**
 * @brief 快速测试命令 - 测试所有舵机
 */
static int cmd_servo_test(int argc, char **argv)
{
    rt_kprintf("===== Servo Test Start =====\n");

    rt_kprintf("1. Moving all servos to middle...\n");
    if (servo_all_middle_speed(SERVO_SPEED_MEDIUM) != 0)
    {
        rt_kprintf("Failed!\n");
        return -1;
    }
    rt_thread_mdelay(2000);

    rt_kprintf("2. Testing each servo individually...\n");
    for (int i = 0; i < 4; i++)
    {
        rt_kprintf("   Servo %d -> MAX\n", i);
        servo_move_by_id_speed(i, SERVO_POS_MAX, SERVO_SPEED_FAST);
        rt_thread_mdelay(1000);

        rt_kprintf("   Servo %d -> MIN\n", i);
        servo_move_by_id_speed(i, SERVO_POS_MIN, SERVO_SPEED_FAST);
        rt_thread_mdelay(1000);

        rt_kprintf("   Servo %d -> MIDDLE\n", i);
        servo_move_by_id_speed(i, SERVO_POS_MIDDLE, SERVO_SPEED_FAST);
        rt_thread_mdelay(1000);
    }

    rt_kprintf("3. Wave motion test...\n");
    servo_preset_wave(2, SERVO_SPEED_FAST);

    rt_kprintf("4. Back to home position...\n");
    servo_preset_home();

    rt_kprintf("===== Servo Test Complete =====\n");
    return 0;
}
MSH_CMD_EXPORT_ALIAS(cmd_servo_test, servo_test, Test all servos);
