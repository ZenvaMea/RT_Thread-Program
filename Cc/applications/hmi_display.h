/*
 * Copyright (c) 2006-2025, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2025-11-15     Cc           HMI Display Driver for TJC UART Screen
 */

#ifndef __HMI_DISPLAY_H__
#define __HMI_DISPLAY_H__

#include <rtthread.h>
#include <rtdevice.h>

/* ==================== Configuration ==================== */
#define HMI_UART_NAME           "uart1"
#define HMI_RINGBUFFER_SIZE     500
#define HMI_RX_THREAD_STACK     2048
#define HMI_RX_THREAD_PRIORITY  15
#define HMI_RX_THREAD_TICK      20

/* ==================== Frame Protocol ==================== */
#define HMI_FRAME_HEADER        0x55
#define HMI_FRAME_TAIL_0        0xFF
#define HMI_FRAME_TAIL_1        0xFF
#define HMI_FRAME_TAIL_2        0xFF
#define HMI_FRAME_LENGTH        7

/* Command Types */
#define HMI_CMD_BUTTON          0x01
#define HMI_CMD_SLIDER_H0       0x02
#define HMI_CMD_SLIDER_H1       0x03

/* ==================== Control Widget Names ==================== */
/* WiFi Status Widgets */
#define HMI_WIFI_STATUS_TEXT    "t_wifi"
#define HMI_WIFI_IP_TEXT        "t_ip"
#define HMI_WIFI_RSSI_NUM       "n_rssi"

/* Servo Control Widgets */
#define HMI_SERVO1_POS_NUM      "n_servo1"
#define HMI_SERVO2_POS_NUM      "n_servo2"
#define HMI_SERVO3_POS_NUM      "n_servo3"
#define HMI_SERVO4_POS_NUM      "n_servo4"

#define HMI_SERVO1_SPEED_SLIDER "h_speed1"
#define HMI_SERVO2_SPEED_SLIDER "h_speed2"
#define HMI_SERVO3_SPEED_SLIDER "h_speed3"
#define HMI_SERVO4_SPEED_SLIDER "h_speed4"

/* System Info Widgets */
#define HMI_CPU_USAGE_TEXT      "t_cpu"
#define HMI_MEMORY_TEXT         "t_mem"
#define HMI_RUNTIME_TEXT        "t_time"

/* Manual Control Buttons */
#define HMI_BTN_SERVO1          "b_servo1"
#define HMI_BTN_SERVO2          "b_servo2"
#define HMI_BTN_SERVO3          "b_servo3"
#define HMI_BTN_SERVO4          "b_servo4"
#define HMI_BTN_WIFI_CONNECT    "b_wifi"

/* ==================== Data Structures ==================== */
typedef struct
{
    uint16_t head;
    uint16_t tail;
    uint16_t length;
    uint8_t  data[HMI_RINGBUFFER_SIZE];
} hmi_ringbuffer_t;

/* ==================== Core API Functions ==================== */
/**
 * @brief Initialize HMI display driver
 * @return RT_EOK on success, error code otherwise
 */
int hmi_init(void);

/**
 * @brief Start HMI receive thread
 * @return RT_EOK on success, error code otherwise
 */
int hmi_start_thread(void);

/**
 * @brief Send raw string to HMI (with frame tail)
 * @param str String to send
 * @return RT_EOK on success, error code otherwise
 */
int hmi_send_string(const char *str);

/**
 * @brief Set text attribute of a widget
 * @param obj_name Widget name (e.g., "t0")
 * @param text Text content
 * @return RT_EOK on success, error code otherwise
 */
int hmi_set_text(const char *obj_name, const char *text);

/**
 * @brief Set value attribute of a widget
 * @param obj_name Widget name (e.g., "n0")
 * @param value Integer value
 * @return RT_EOK on success, error code otherwise
 */
int hmi_set_value(const char *obj_name, int value);

/**
 * @brief Set button state (simulated click)
 * @param btn_name Button name (e.g., "b0")
 * @param pressed 1 = pressed, 0 = released
 * @return RT_EOK on success, error code otherwise
 */
int hmi_set_button_state(const char *btn_name, int pressed);

/* ==================== High-level API Functions ==================== */
/**
 * @brief Update WiFi connection status display
 * @param ssid WiFi SSID, NULL if disconnected
 * @param ip IP address string, NULL if not assigned
 * @param rssi Signal strength in dBm
 */
void hmi_update_wifi_status(const char *ssid, const char *ip, int rssi);

/**
 * @brief Update servo position display
 * @param servo_id Servo ID (1-4)
 * @param position Servo position (0-4095)
 */
void hmi_update_servo_pos(int servo_id, int position);

/**
 * @brief Update servo speed display
 * @param servo_id Servo ID (1-4)
 * @param speed Speed value (0-100)
 */
void hmi_update_servo_speed(int servo_id, int speed);

/**
 * @brief Update system CPU usage display
 * @param percent CPU usage percentage (0-100)
 */
void hmi_update_cpu_usage(int percent);

/**
 * @brief Update system memory info display
 * @param used_kb Used memory in KB
 * @param total_kb Total memory in KB
 */
void hmi_update_memory_info(int used_kb, int total_kb);

/**
 * @brief Update system runtime display
 * @param seconds Runtime in seconds
 */
void hmi_update_runtime(int seconds);

/* ==================== Callback Functions (Implemented by User) ==================== */
/**
 * @brief Callback when button is clicked on HMI screen
 * @param button_id Button ID from screen
 * @param state 1 = pressed, 0 = released
 * @note User should implement this function in hmi_callbacks.c
 */
extern void hmi_on_button_click(int button_id, int state);

/**
 * @brief Callback when slider value changes on HMI screen
 * @param slider_id Slider ID (0=h0, 1=h1)
 * @param value Slider value (0-255)
 * @note User should implement this function in hmi_callbacks.c
 */
extern void hmi_on_slider_change(int slider_id, int value);

/* ==================== MSH Commands ==================== */
#ifdef RT_USING_FINSH
int hmi_test(int argc, char **argv);
#endif

#endif /* __HMI_DISPLAY_H__ */
