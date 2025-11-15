/*
 * Copyright (c) 2006-2025, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2025-01-14     Cc           HTTP客户端模块
 */

#ifndef __SERVO_HTTP_CLIENT_H__
#define __SERVO_HTTP_CLIENT_H__

#include <rtthread.h>

/* HTTP响应结构 */
typedef struct {
    int status_code;        /* HTTP状态码 */
    char *body;            /* 响应内容 */
    int body_len;          /* 响应内容长度 */
} http_response_t;

/**
 * @brief 初始化HTTP客户端
 * @return 0: 成功, -1: 失败
 */
int http_client_init(void);

/**
 * @brief 发送HTTP GET请求
 * @param url 完整的URL地址
 * @param response 响应结构体指针(可为NULL)
 * @param timeout_ms 超时时间(毫秒)
 * @return 0: 成功, -1: 失败
 */
int http_get(const char *url, http_response_t *response, int timeout_ms);

/**
 * @brief 发送简单的HTTP GET请求(不获取响应内容)
 * @param url 完整的URL地址
 * @return 0: 成功, -1: 失败
 */
int http_get_simple(const char *url);

/**
 * @brief 释放HTTP响应结构体
 * @param response 响应结构体指针
 */
void http_response_free(http_response_t *response);

#endif /* __SERVO_HTTP_CLIENT_H__ */
