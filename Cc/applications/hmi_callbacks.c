/*
 * Copyright (c) 2006-2025, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2025-11-15     Cc           HMI Event Callbacks (User Template)
 */

#include "hmi_display.h"
#include "servo_advanced.h"
#include "wifi_manager.h"

#define DBG_TAG "hmi.cb"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

/* ==================== Button Event Callback ==================== */
/**
 * @brief Handle button click events from HMI screen
 *
 * @param button_id Button identifier from HMI screen
 *                  You can define your own button ID mapping
 * @param state     Button state: 1 = pressed, 0 = released
 *
 * @note Template Implementation - Modify this function for your needs!
 *
 * Example button ID mapping:
 *   0 - LED control button
 *   1 - Servo 1 control
 *   2 - Servo 2 control
 *   3 - Servo 3 control
 *   4 - Servo 4 control
 *   5 - WiFi connect/disconnect
 */
void hmi_on_button_click(int button_id, int state)
{
    LOG_I("Button clicked: ID=%d, State=%s", button_id, state ? "PRESSED" : "RELEASED");

    /* Only process button press events (ignore release) */
    if (state != 1)
    {
        return;
    }

    /* Handle button events based on ID */
    switch (button_id)
    {
        case 0:
            /* Example: LED toggle button */
            LOG_I("LED toggle button pressed");
            /* TODO: Add your LED control code here */
            hmi_set_text("t_msg", "LED Toggled");
            break;

        case 1:
            /* Example: Move Servo 1 to middle position */
            LOG_I("Servo 1 control button pressed");
            servo_move_by_id_speed(1, 2048, SERVO_SPEED_MEDIUM);
            hmi_set_text("t_msg", "Servo 1 -> Mid");
            break;

        case 2:
            /* Example: Move Servo 2 to middle position */
            LOG_I("Servo 2 control button pressed");
            servo_move_by_id_speed(2, 2048, SERVO_SPEED_MEDIUM);
            hmi_set_text("t_msg", "Servo 2 -> Mid");
            break;

        case 3:
            /* Example: Move Servo 3 to middle position */
            LOG_I("Servo 3 control button pressed");
            servo_move_by_id_speed(3, 2048, SERVO_SPEED_MEDIUM);
            hmi_set_text("t_msg", "Servo 3 -> Mid");
            break;

        case 4:
            /* Example: Move Servo 4 to middle position */
            LOG_I("Servo 4 control button pressed");
            servo_move_by_id_speed(4, 2048, SERVO_SPEED_MEDIUM);
            hmi_set_text("t_msg", "Servo 4 -> Mid");
            break;

        case 5:
            /* Example: WiFi connect/disconnect toggle */
            LOG_I("WiFi button pressed");
            wifi_status_t status = wifi_get_status();
            if (status == WIFI_STATUS_CONNECTED)
            {
                LOG_I("Disconnecting WiFi...");
                hmi_set_text("t_msg", "WiFi Disconnecting");
                /* TODO: Add WiFi disconnect code if needed */
            }
            else
            {
                LOG_I("Connecting WiFi...");
                hmi_set_text("t_msg", "WiFi Connecting");
                /* TODO: Add WiFi connect code */
                // wifi_connect("YourSSID", "YourPassword");
            }
            break;

        case 10:
            /* Example: All servos to home position */
            LOG_I("Home button pressed - All servos to center");
            servo_preset_home();
            hmi_set_text("t_msg", "All Servos Home");
            break;

        case 11:
            /* Example: Execute wave action */
            LOG_I("Wave button pressed");
            //servo_preset_wave();
            hmi_set_text("t_msg", "Wave Action");
            break;

        default:
            LOG_W("Unknown button ID: %d", button_id);
            break;
    }
}

/* ==================== Slider Event Callback ==================== */
/**
 * @brief Handle slider value change events from HMI screen
 *
 * @param slider_id Slider identifier (0=h0, 1=h1, etc.)
 * @param value     Slider value (typically 0-255 or 0-100)
 *
 * @note Template Implementation - Modify this function for your needs!
 *
 * Example slider ID mapping:
 *   0 (h0) - Servo 1 speed control
 *   1 (h1) - Servo 2 speed control
 *   2 (h2) - Brightness control
 *   3 (h3) - Volume control
 */
void hmi_on_slider_change(int slider_id, int value)
{
    LOG_I("Slider changed: ID=%d, Value=%d", slider_id, value);

    switch (slider_id)
    {
        case 0:
            /* Example: h0 slider controls Servo 1 speed */
            LOG_I("Servo 1 speed slider: %d", value);
            /* Map slider value (0-255) to servo speed (1-4) */
            int speed1 = (value / 64) + 1;  /* Result: 1, 2, 3, or 4 */
            if (speed1 > 4) speed1 = 4;

            /* Update speed display */
            hmi_update_servo_speed(1, value);

            /* TODO: Store speed for next servo movement */
            /* servo_set_default_speed(1, speed1); */
            break;

        case 1:
            /* Example: h1 slider controls Servo 2 speed */
            LOG_I("Servo 2 speed slider: %d", value);
            int speed2 = (value / 64) + 1;
            if (speed2 > 4) speed2 = 4;
            hmi_update_servo_speed(2, value);
            break;

        case 2:
            /* Example: h2 slider controls screen brightness */
            LOG_I("Brightness slider: %d", value);
            char brightness_cmd[32];
            rt_snprintf(brightness_cmd, sizeof(brightness_cmd), "dim=%d", value);
            hmi_send_string(brightness_cmd);
            break;

        case 3:
            /* Example: h3 slider controls system parameter */
            LOG_I("Custom parameter slider: %d", value);
            /* TODO: Add your custom slider handling code */
            break;

        default:
            LOG_W("Unknown slider ID: %d", slider_id);
            break;
    }
}

/* ==================== User Utility Functions ==================== */

/**
 * @brief Custom function: Update all servo positions on HMI
 *
 * @note This is a user-defined utility function example
 */
void hmi_update_all_servos(void)
{
    /* Example: Read and display all servo positions */
    /* You would need to implement servo position reading first */

    LOG_I("Updating all servo positions on HMI");

    /* Placeholder values - replace with actual servo position reading */
    hmi_update_servo_pos(1, 2048);
    hmi_update_servo_pos(2, 2048);
    hmi_update_servo_pos(3, 2048);
    hmi_update_servo_pos(4, 2048);
}

/**
 * @brief Custom function: Send welcome message to HMI
 */
void hmi_show_welcome_message(void)
{
    hmi_set_text("t_msg", "System Ready!");
    hmi_set_text("t_wifi", "WiFi: Initializing");
    hmi_set_text("t_cpu", "CPU: --");
    hmi_set_text("t_mem", "Mem: --");
    hmi_set_text("t_time", "00:00:00");

    LOG_I("Welcome message sent to HMI");
}

/* ==================== Template Usage Examples ==================== */
/*
 * EXAMPLE 1: Control servo to specific position when button pressed
 * -----------------------------------------------------------------
 * In hmi_on_button_click():
 *
 *     case 20:  // Button for Servo 1 max position
 *         servo_move_by_id_speed(1, 4095, SERVO_SPEED_FAST);
 *         hmi_set_text("t_msg", "Servo 1 -> MAX");
 *         break;
 *
 * EXAMPLE 2: Read servo position and update HMI display
 * -------------------------------------------------------
 * If you have a function to read servo position:
 *
 *     int pos = servo_read_position(1);
 *     hmi_update_servo_pos(1, pos);
 *
 * EXAMPLE 3: Control servo position with slider
 * ----------------------------------------------
 * In hmi_on_slider_change():
 *
 *     case 4:  // h4 slider for Servo 1 position
 *         // Map slider 0-255 to servo 0-4095
 *         int pos = (value * 4095) / 255;
 *         servo_move_by_id_speed(1, pos, SERVO_SPEED_SLOW);
 *         hmi_update_servo_pos(1, pos);
 *         break;
 *
 * EXAMPLE 4: Multi-button servo control panel
 * --------------------------------------------
 *     case 30: servo_move_by_id_speed(1, 0, SERVO_SPEED_MEDIUM); break;      // Min
 *     case 31: servo_move_by_id_speed(1, 2048, SERVO_SPEED_MEDIUM); break;   // Mid
 *     case 32: servo_move_by_id_speed(1, 4095, SERVO_SPEED_MEDIUM); break;   // Max
 *
 * EXAMPLE 5: Sequential action on button press
 * ---------------------------------------------
 *     case 40:  // Sequential action button
 *         servo_move_by_id_speed(1, 1000, SERVO_SPEED_SLOW);
 *         rt_thread_mdelay(500);
 *         servo_move_by_id_speed(2, 3000, SERVO_SPEED_SLOW);
 *         rt_thread_mdelay(500);
 *         servo_move_by_id_speed(3, 2000, SERVO_SPEED_SLOW);
 *         hmi_set_text("t_msg", "Sequence Done");
 *         break;
 */
