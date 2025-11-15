/*
 * Copyright (c) 2006-2025, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2025-01-14     Claude       HTTP客户端实现
 */

#include "servo_http_client.h"
#include <rtthread.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#define DBG_TAG "http.client"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

#define HTTP_RECV_BUF_SIZE 2048
#define HTTP_SEND_BUF_SIZE 512
#define HTTP_DEFAULT_TIMEOUT 3000

/**
 * @brief 初始化HTTP客户端
 */
int http_client_init(void)
{
    LOG_I("HTTP client initialized");
    return 0;
}

/**
 * @brief 解析URL
 */
static int parse_url(const char *url, char *host, int *port, char *path)
{
    const char *p1, *p2, *p3;

    /* 检查是否以http://开头 */
    if (strncmp(url, "http://", 7) == 0)
    {
        p1 = url + 7;
    }
    else
    {
        p1 = url;
    }

    /* 查找主机名结束位置 */
    p2 = strchr(p1, ':');
    p3 = strchr(p1, '/');

    if (p2 && (!p3 || p2 < p3))
    {
        /* 有端口号 */
        int host_len = p2 - p1;
        strncpy(host, p1, host_len);
        host[host_len] = '\0';

        /* 解析端口号 */
        *port = atoi(p2 + 1);

        /* 解析路径 */
        p3 = strchr(p2, '/');
        if (p3)
        {
            strcpy(path, p3);
        }
        else
        {
            strcpy(path, "/");
        }
    }
    else if (p3)
    {
        /* 没有端口号,使用默认端口80 */
        int host_len = p3 - p1;
        strncpy(host, p1, host_len);
        host[host_len] = '\0';
        *port = 80;
        strcpy(path, p3);
    }
    else
    {
        /* 只有主机名 */
        strcpy(host, p1);
        *port = 80;
        strcpy(path, "/");
    }

    return 0;
}

/**
 * @brief 发送HTTP GET请求
 */
int http_get(const char *url, http_response_t *response, int timeout_ms)
{
    int sock = -1;
    struct sockaddr_in server_addr;
    struct hostent *host_entry;
    char host[64] = {0};
    char path[256] = {0};
    int port = 80;
    char *send_buf = RT_NULL;
    char *recv_buf = RT_NULL;
    int ret = -1;
    int timeout_sec;

    if (url == RT_NULL)
    {
        LOG_E("URL is NULL");
        return -1;
    }

    if (timeout_ms <= 0)
    {
        timeout_ms = HTTP_DEFAULT_TIMEOUT;
    }

    /* 解析URL */
    if (parse_url(url, host, &port, path) != 0)
    {
        LOG_E("Parse URL failed");
        return -1;
    }

    LOG_D("Host: %s, Port: %d, Path: %s", host, port, path);

    /* 创建socket */
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0)
    {
        LOG_E("Create socket failed");
        return -1;
    }

    /* 设置超时(转换为秒) */
    timeout_sec = (timeout_ms + 999) / 1000;
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &timeout_sec, sizeof(timeout_sec));
    setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, &timeout_sec, sizeof(timeout_sec));

    /* 解析主机名 */
    host_entry = gethostbyname(host);
    if (host_entry == RT_NULL)
    {
        LOG_E("Get host by name failed");
        goto exit;
    }

    /* 配置服务器地址 */
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr = *((struct in_addr *)host_entry->h_addr);
    memset(&(server_addr.sin_zero), 0, sizeof(server_addr.sin_zero));

    /* 连接服务器 */
    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(struct sockaddr)) < 0)
    {
        LOG_E("Connect to server failed");
        goto exit;
    }

    /* 分配发送缓冲区 */
    send_buf = rt_malloc(HTTP_SEND_BUF_SIZE);
    if (send_buf == RT_NULL)
    {
        LOG_E("Malloc send buffer failed");
        goto exit;
    }

    /* 构建HTTP请求 */
    rt_snprintf(send_buf, HTTP_SEND_BUF_SIZE,
                "GET %s HTTP/1.1\r\n"
                "Host: %s\r\n"
                "Connection: close\r\n"
                "\r\n",
                path, host);

    /* 发送HTTP请求 */
    if (send(sock, send_buf, strlen(send_buf), 0) < 0)
    {
        LOG_E("Send HTTP request failed");
        goto exit;
    }

    /* 如果需要接收响应 */
    if (response != RT_NULL)
    {
        /* 分配接收缓冲区 */
        recv_buf = rt_malloc(HTTP_RECV_BUF_SIZE);
        if (recv_buf == RT_NULL)
        {
            LOG_E("Malloc recv buffer failed");
            goto exit;
        }

        /* 接收响应 */
        int recv_len = recv(sock, recv_buf, HTTP_RECV_BUF_SIZE - 1, 0);
        if (recv_len > 0)
        {
            recv_buf[recv_len] = '\0';

            /* 解析状态码 */
            char *status_line = strstr(recv_buf, "HTTP/1.");
            if (status_line)
            {
                sscanf(status_line, "HTTP/1.%*d %d", &response->status_code);
            }

            /* 查找响应体 */
            char *body_start = strstr(recv_buf, "\r\n\r\n");
            if (body_start)
            {
                body_start += 4;
                int body_len = recv_len - (body_start - recv_buf);
                response->body = rt_malloc(body_len + 1);
                if (response->body)
                {
                    memcpy(response->body, body_start, body_len);
                    response->body[body_len] = '\0';
                    response->body_len = body_len;
                }
            }

            LOG_D("HTTP Status: %d", response->status_code);
        }
    }

    ret = 0;
    LOG_D("HTTP request success");

exit:
    if (send_buf)
    {
        rt_free(send_buf);
    }
    if (recv_buf)
    {
        rt_free(recv_buf);
    }
    if (sock >= 0)
    {
        closesocket(sock);
    }

    return ret;
}

/**
 * @brief 发送简单的HTTP GET请求
 */
int http_get_simple(const char *url)
{
    return http_get(url, RT_NULL, HTTP_DEFAULT_TIMEOUT);
}

/**
 * @brief 释放HTTP响应结构体
 */
void http_response_free(http_response_t *response)
{
    if (response && response->body)
    {
        rt_free(response->body);
        response->body = RT_NULL;
        response->body_len = 0;
    }
}
