/*
 * Copyright (c) 2006-2025, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2025-11-15     Cc           HMI Display Driver Implementation
 */

#include "hmi_display.h"
#include <stdio.h>
#include <string.h>

#define DBG_TAG "hmi"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

/* ==================== Private Variables ==================== */
static rt_device_t hmi_serial = RT_NULL;
static hmi_ringbuffer_t ring_buffer;
static rt_sem_t rx_sem = RT_NULL;
static rt_thread_t rx_thread = RT_NULL;

/* ==================== Ring Buffer Operations ==================== */
static void ringbuffer_init(void)
{
    ring_buffer.head = 0;
    ring_buffer.tail = 0;
    ring_buffer.length = 0;
    rt_memset(ring_buffer.data, 0, HMI_RINGBUFFER_SIZE);
}

static void ringbuffer_write_byte(uint8_t byte)
{
    if (ring_buffer.length < HMI_RINGBUFFER_SIZE)
    {
        ring_buffer.data[ring_buffer.tail] = byte;
        ring_buffer.tail = (ring_buffer.tail + 1) % HMI_RINGBUFFER_SIZE;
        ring_buffer.length++;
    }
    else
    {
        LOG_W("Ring buffer overflow!");
    }
}

static uint8_t ringbuffer_read_byte(uint16_t position)
{
    if (position < ring_buffer.length)
    {
        uint16_t index = (ring_buffer.head + position) % HMI_RINGBUFFER_SIZE;
        return ring_buffer.data[index];
    }
    return 0;
}

static void ringbuffer_delete_bytes(uint16_t count)
{
    if (count > ring_buffer.length)
    {
        count = ring_buffer.length;
    }

    ring_buffer.head = (ring_buffer.head + count) % HMI_RINGBUFFER_SIZE;
    ring_buffer.length -= count;
}

static uint16_t ringbuffer_get_length(void)
{
    return ring_buffer.length;
}

/* Convenient macros for ring buffer access */
#define usize()         ringbuffer_get_length()
#define u(x)            ringbuffer_read_byte(x)
#define udelete(x)      ringbuffer_delete_bytes(x)

/* ==================== UART Operations ==================== */
static rt_err_t hmi_uart_rx_indicate(rt_device_t dev, rt_size_t size)
{
    uint8_t ch;

    /* Read all available data */
    while (rt_device_read(dev, 0, &ch, 1) > 0)
    {
        ringbuffer_write_byte(ch);
    }

    /* Release semaphore to wake up receive thread */
    if (rx_sem != RT_NULL)
    {
        rt_sem_release(rx_sem);
    }

    return RT_EOK;
}

/* ==================== Frame Protocol Operations ==================== */
/**
 * @brief Send raw data with frame tail (0xFF 0xFF 0xFF)
 */
static int hmi_send_raw(const char *data, rt_size_t len)
{
    if (hmi_serial == RT_NULL)
    {
        LOG_E("HMI serial not initialized");
        return -RT_ERROR;
    }

    /* Send data */
    rt_device_write(hmi_serial, 0, data, len);

    /* Send frame tail */
    uint8_t tail[3] = {HMI_FRAME_TAIL_0, HMI_FRAME_TAIL_1, HMI_FRAME_TAIL_2};
    rt_device_write(hmi_serial, 0, tail, 3);

    return RT_EOK;
}

int hmi_send_string(const char *str)
{
    if (str == RT_NULL)
    {
        return -RT_ERROR;
    }

    return hmi_send_raw(str, rt_strlen(str));
}

int hmi_set_text(const char *obj_name, const char *text)
{
    char buffer[128];

    if (obj_name == RT_NULL || text == RT_NULL)
    {
        return -RT_ERROR;
    }

    rt_snprintf(buffer, sizeof(buffer), "%s.txt=\"%s\"", obj_name, text);
    return hmi_send_string(buffer);
}

int hmi_set_value(const char *obj_name, int value)
{
    char buffer[64];

    if (obj_name == RT_NULL)
    {
        return -RT_ERROR;
    }

    rt_snprintf(buffer, sizeof(buffer), "%s.val=%d", obj_name, value);
    return hmi_send_string(buffer);
}

int hmi_set_button_state(const char *btn_name, int pressed)
{
    char buffer[64];

    if (btn_name == RT_NULL)
    {
        return -RT_ERROR;
    }

    rt_snprintf(buffer, sizeof(buffer), "click %s,%d", btn_name, pressed ? 1 : 0);
    return hmi_send_string(buffer);
}

/* ==================== Frame Parsing and Processing ==================== */
static void hmi_process_frame(void)
{
    uint8_t cmd_type, data2, data3;

    /* Parse frame */
    cmd_type = u(1);
    data2 = u(2);
    data3 = u(3);

    /* Process based on command type */
    switch (cmd_type)
    {
        case HMI_CMD_BUTTON:
            LOG_D("Button event: ID=%d, State=%d", data2, data3);
            hmi_on_button_click(data2, data3);
            break;

        case HMI_CMD_SLIDER_H0:
            LOG_D("Slider h0 event: Value=%d", data2);
            hmi_on_slider_change(0, data2);
            break;

        case HMI_CMD_SLIDER_H1:
            LOG_D("Slider h1 event: Value=%d", data2);
            hmi_on_slider_change(1, data2);
            break;

        default:
            LOG_W("Unknown command type: 0x%02X", cmd_type);
            break;
    }
}

/* ==================== Receive Thread ==================== */
static void hmi_rx_thread_entry(void *parameter)
{
    LOG_I("HMI receive thread started");

    while (1)
    {
        /* Wait for data arrival (with timeout) */
        rt_sem_take(rx_sem, RT_WAITING_FOREVER);

        /* Process all complete frames in buffer */
        while (usize() >= HMI_FRAME_LENGTH)
        {
            /* Validate frame header and tail */
            if (u(0) == HMI_FRAME_HEADER &&
                u(4) == HMI_FRAME_TAIL_0 &&
                u(5) == HMI_FRAME_TAIL_1 &&
                u(6) == HMI_FRAME_TAIL_2)
            {
                /* Valid frame found, process it */
                hmi_process_frame();

                /* Delete processed frame */
                udelete(HMI_FRAME_LENGTH);
            }
            else
            {
                /* Invalid frame header, skip one byte and continue searching */
                udelete(1);
                break;
            }
        }
    }
}

/* ==================== High-level API Functions ==================== */
void hmi_update_wifi_status(const char *ssid, const char *ip, int rssi)
{
    if (ssid != RT_NULL)
    {
        hmi_set_text(HMI_WIFI_STATUS_TEXT, ssid);
    }
    else
    {
        hmi_set_text(HMI_WIFI_STATUS_TEXT, "Disconnected");
    }

    if (ip != RT_NULL)
    {
        hmi_set_text(HMI_WIFI_IP_TEXT, ip);
    }
    else
    {
        hmi_set_text(HMI_WIFI_IP_TEXT, "0.0.0.0");
    }

    hmi_set_value(HMI_WIFI_RSSI_NUM, rssi);
}

void hmi_update_servo_pos(int servo_id, int position)
{
    const char *widget_names[] = {
        HMI_SERVO1_POS_NUM,
        HMI_SERVO2_POS_NUM,
        HMI_SERVO3_POS_NUM,
        HMI_SERVO4_POS_NUM
    };

    if (servo_id >= 1 && servo_id <= 4)
    {
        hmi_set_value(widget_names[servo_id - 1], position);
    }
}

void hmi_update_servo_speed(int servo_id, int speed)
{
    const char *widget_names[] = {
        HMI_SERVO1_SPEED_SLIDER,
        HMI_SERVO2_SPEED_SLIDER,
        HMI_SERVO3_SPEED_SLIDER,
        HMI_SERVO4_SPEED_SLIDER
    };

    if (servo_id >= 1 && servo_id <= 4)
    {
        hmi_set_value(widget_names[servo_id - 1], speed);
    }
}

void hmi_update_cpu_usage(int percent)
{
    char buffer[32];
    rt_snprintf(buffer, sizeof(buffer), "CPU: %d%%", percent);
    hmi_set_text(HMI_CPU_USAGE_TEXT, buffer);
}

void hmi_update_memory_info(int used_kb, int total_kb)
{
    char buffer[64];
    rt_snprintf(buffer, sizeof(buffer), "Mem: %d/%d KB", used_kb, total_kb);
    hmi_set_text(HMI_MEMORY_TEXT, buffer);
}

void hmi_update_runtime(int seconds)
{
    char buffer[32];
    int hours = seconds / 3600;
    int minutes = (seconds % 3600) / 60;
    int secs = seconds % 60;

    rt_snprintf(buffer, sizeof(buffer), "%02d:%02d:%02d", hours, minutes, secs);
    hmi_set_text(HMI_RUNTIME_TEXT, buffer);
}

/* ==================== Initialization ==================== */
int hmi_init(void)
{
    /* Find UART device */
    hmi_serial = rt_device_find(HMI_UART_NAME);
    if (hmi_serial == RT_NULL)
    {
        LOG_E("Cannot find UART device: %s", HMI_UART_NAME);
        return -RT_ERROR;
    }

    /* Open UART device in interrupt receive mode */
    rt_err_t ret = rt_device_open(hmi_serial, RT_DEVICE_FLAG_INT_RX);
    if (ret != RT_EOK)
    {
        LOG_E("Failed to open UART device: %d", ret);
        return ret;
    }

    /* Set receive callback */
    rt_device_set_rx_indicate(hmi_serial, hmi_uart_rx_indicate);

    /* Initialize ring buffer */
    ringbuffer_init();

    /* Create semaphore for receive notification */
    rx_sem = rt_sem_create("hmi_rx", 0, RT_IPC_FLAG_FIFO);
    if (rx_sem == RT_NULL)
    {
        LOG_E("Failed to create semaphore");
        rt_device_close(hmi_serial);
        return -RT_ERROR;
    }

    LOG_I("HMI display driver initialized successfully");
    return RT_EOK;
}

int hmi_start_thread(void)
{
    /* Create receive thread */
    rx_thread = rt_thread_create("hmi_rx",
                                  hmi_rx_thread_entry,
                                  RT_NULL,
                                  HMI_RX_THREAD_STACK,
                                  HMI_RX_THREAD_PRIORITY,
                                  HMI_RX_THREAD_TICK);

    if (rx_thread == RT_NULL)
    {
        LOG_E("Failed to create receive thread");
        return -RT_ERROR;
    }

    /* Start thread */
    rt_thread_startup(rx_thread);

    LOG_I("HMI receive thread started");
    return RT_EOK;
}

/* ==================== MSH Test Commands ==================== */
#ifdef RT_USING_FINSH
#include <finsh.h>

int hmi_test(int argc, char **argv)
{
    if (argc < 2)
    {
        rt_kprintf("Usage:\n");
        rt_kprintf("  hmi_test text <obj> <text>   - Set text\n");
        rt_kprintf("  hmi_test value <obj> <val>   - Set value\n");
        rt_kprintf("  hmi_test button <btn> <0|1>  - Click button\n");
        rt_kprintf("  hmi_test wifi                - Test WiFi display\n");
        rt_kprintf("  hmi_test servo <id> <pos>    - Test servo display\n");
        rt_kprintf("  hmi_test system              - Test system info\n");
        return -1;
    }

    if (rt_strcmp(argv[1], "text") == 0 && argc >= 4)
    {
        hmi_set_text(argv[2], argv[3]);
        rt_kprintf("Set %s.txt = \"%s\"\n", argv[2], argv[3]);
    }
    else if (rt_strcmp(argv[1], "value") == 0 && argc >= 4)
    {
        int value = atoi(argv[3]);
        hmi_set_value(argv[2], value);
        rt_kprintf("Set %s.val = %d\n", argv[2], value);
    }
    else if (rt_strcmp(argv[1], "button") == 0 && argc >= 4)
    {
        int state = atoi(argv[3]);
        hmi_set_button_state(argv[2], state);
        rt_kprintf("Click %s = %d\n", argv[2], state);
    }
    else if (rt_strcmp(argv[1], "wifi") == 0)
    {
        hmi_update_wifi_status("Test_WiFi", "192.168.1.100", -45);
        rt_kprintf("WiFi status updated\n");
    }
    else if (rt_strcmp(argv[1], "servo") == 0 && argc >= 4)
    {
        int id = atoi(argv[2]);
        int pos = atoi(argv[3]);
        hmi_update_servo_pos(id, pos);
        rt_kprintf("Servo %d position = %d\n", id, pos);
    }
    else if (rt_strcmp(argv[1], "system") == 0)
    {
        hmi_update_cpu_usage(25);
        hmi_update_memory_info(128, 456);
        hmi_update_runtime(3661);
        rt_kprintf("System info updated\n");
    }
    else
    {
        rt_kprintf("Invalid command\n");
        return -1;
    }

    return 0;
}
MSH_CMD_EXPORT(hmi_test, HMI display test commands);
#endif
