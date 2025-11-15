# RT-Thread 外骨骼手指控制系统

<div align="center">

![RT-Thread](https://img.shields.io/badge/RT--Thread-5.1.0-blue)
![Platform](https://img.shields.io/badge/Platform-STM32H7RS-green)
![License](https://img.shields.io/badge/License-Apache--2.0-orange)
![Status](https://img.shields.io/badge/Status-In%20Development-yellow)

**基于RT-Thread RTOS的肌电信号驱动外骨骼手指控制系统**

[快速开始](#-快速开始) • [命令参考](命令参考手册.md) • [开发日志](开发日志.md) • [功能特性](#-功能特性)

</div>

---

## 📖 项目简介

本项目是一个基于RT-Thread RTOS的外骨骼手指控制系统，通过肌电信号采集驱动4个SCSCL舵机，实现外骨骼手指的精确控制。系统集成WiFi通信、串口屏显示、实时舵机控制等功能模块，适用于康复辅助、人机交互等应用场景。

### 🎯 核心目标

- 🦾 **外骨骼手指辅助** - 通过肌电信号控制机械手指运动
- 💪 **肌电信号驱动** - 实时采集、处理肌电信号
- 🖐️ **四指独立控制** - 每个舵机独立驱动一根手指
- 📊 **实时状态显示** - 串口屏显示WiFi、舵机、系统状态

---

## ✨ 功能特性

### 已完成功能 ✅

#### 1. WiFi通信模块
- ✅ ESP32 AP模式连接（SSID: ESP32_DEV）
- ✅ 事件驱动的连接状态管理
- ✅ 自动IP地址获取和显示
- ✅ 完整的WiFi命令集（connect_esp32, wifi_join, wifi_leave, wifi_info）

#### 2. 舵机控制系统
- ✅ **基础控制** - 单舵机位置、速度、扭矩控制
- ✅ **高级控制** - 多舵机协同、按ID直接控制、4级速度调节
- ✅ **批量操作** - 同时控制多个舵机移动
- ✅ **动作序列** - 可编程的复杂动作组合
- ✅ **预设动作** - 回中位、波浪、依次动作等
- ✅ **线程安全** - 互斥锁保护，支持多线程调用

#### 3. HMI串口屏显示
- ✅ **双向通信** - STM32发送命令、接收按钮/滑块事件
- ✅ **实时显示** - WiFi状态、舵机位置、系统信息
- ✅ **环形缓冲** - 高效的中断安全数据管理
- ✅ **事件回调** - 用户可定制的按钮和滑块事件处理
- ✅ **自动更新** - 状态变化时自动刷新显示

#### 4. HTTP客户端
- ✅ 基于Socket的HTTP GET实现
- ✅ URL解析和连接管理
- ✅ 超时控制和错误处理

### 规划功能 ⏳

#### 阶段3：肌电信号控制（即将开始）
- ⏳ UART肌电臂环数据采集
- ⏳ 实时信号处理和滤波
- ⏳ 肌电-舵机映射算法
- ⏳ 四通道独立控制
- ⏳ 自适应阈值校准

#### 阶段4：系统集成优化
- ⏳ 三模式切换（手动/肌电/演示）
- ⏳ 参数配置界面
- ⏳ 数据记录和回放
- ⏳ 故障诊断和安全保护

#### 阶段5：测试与完善
- ⏳ 功能测试和性能测试
- ⏳ 稳定性测试
- ⏳ 用户体验优化

---

## 🏗️ 系统架构

```
┌─────────────────────────────────────────────────────────────┐
│                       控制系统架构                            │
└─────────────────────────────────────────────────────────────┘

  肌电臂环                 ART-PI主控           ESP32驱动板
  (传感器)                (STM32H7R)            (WiFi AP)
     │                        │                     │
     │  肌电信号(UART)         │                     │
     ├───────────────────────>│                     │
     │                        │                     │
     │                        │   WiFi连接          │
     │                        │<───────────────────>│
     │                        │   HTTP命令          │
     │                        │                     │
     │                        │                     │  SCSCL协议
     │                        │                     ├───────────> 舵机0 (食指)
     │                        │                     ├───────────> 舵机1 (中指)
     │      串口屏             │                     ├───────────> 舵机2 (无名指)
     │     (显示)              │                     └───────────> 舵机3 (小指)
     │        │                │
     │        │<───────────────┤
     │        │   UART显示数据  │
     │                        │
     │                   电源供电系统
     │                    (8.4V)
     │                        │
     └────────────────────────┴──────────────────────────────────
                          (共地)
```

---

## 💻 硬件配置

### 主控板
- **型号**: STM32H7RS7L8H6
- **架构**: ARM Cortex-M7 @480MHz
- **Flash**: 2MB
- **RAM**: 620KB
- **开发板**: ART-PI H7RS

### WiFi模块
- **芯片**: CYW43438
- **模式**: STA模式（连接ESP32 AP）
- **协议**: lwIP网络栈
- **通信**: HTTP客户端

### 舵机系统
- **型号**: SCSCL舵机 × 4
- **电压**: 8.4V (2S锂电池)
- **通信**: UART串口总线（通过ESP32转发）
- **控制**: 位置、速度、扭矩

### 串口屏
- **型号**: TJC/迪文串口屏
- **接口**: UART1 (PA9/PA10)
- **波特率**: 115200 baud
- **协议**: ASCII命令 + 0xFF×3

### 供电系统
- **输入**: 12V电源适配器
- **舵机**: DC-DC降压至8.4V（3A）
- **主控**: 板载稳压器（5V/3.3V）

---

## 🚀 快速开始

### 1. 环境准备

#### 软件要求
- RT-Thread Studio 或 MDK-ARM
- RT-Thread 5.1.0
- Git

#### 硬件连接
```
舵机供电：
12V电源 → DC-DC降压模块 → 8.4V → 舵机VCC (共地)

串口屏连接：
STM32 PA9  → 串口屏 RX
STM32 PA10 ← 串口屏 TX
STM32 GND  → 串口屏 GND
STM32 5V   → 串口屏 VCC
```

### 2. 克隆项目

```bash
git clone https://github.com/ZenvaMea/RT_Thread-Program.git
cd RT_Thread-Program/Cc
```

### 3. 编译下载

1. 使用RT-Thread Studio打开项目
2. 编译工程
3. 下载到STM32H7RS开发板
4. 复位运行

### 4. 连接WiFi

```shell
msh /> connect_esp32
[I/wifi.mgr] Connecting to ESP32 AP...
[I/wifi.mgr] SSID: ESP32_DEV
[I/wifi.mgr] WiFi connected successfully!
[I/wifi.mgr] IP Address: 192.168.4.10
```

### 5. 测试舵机

```shell
# 控制舵机0到中间位置，中速
msh /> serv move 0 0 2

# 所有舵机回中位
msh /> serv all_mid 3

# 执行波浪动作
msh /> serv wave 3 2

# 运行完整测试
msh /> servo_test
```

### 6. 查看状态

```shell
# WiFi状态
msh /> wifi_info

# 系统线程
msh /> list_thread

# 内存使用
msh /> free
```

---

## 📚 命令参考

### WiFi命令
| 命令 | 功能 | 示例 |
|------|------|------|
| `connect_esp32` | 连接ESP32_DEV | `connect_esp32` |
| `wifi_join` | 连接指定WiFi | `wifi_join MyWiFi 12345678` |
| `wifi_leave` | 断开WiFi | `wifi_leave` |
| `wifi_info` | 查看WiFi状态 | `wifi_info` |

### 舵机控制命令
| 命令 | 功能 | 示例 |
|------|------|------|
| `serv move` | 控制单个舵机 | `serv move 0 0 2` |
| `serv all_mid` | 所有舵机回中位 | `serv all_mid 3` |
| `serv all_stop` | 停止所有舵机 | `serv all_stop` |
| `serv all_ton/toff` | 扭矩开关 | `serv all_ton` |
| `serv wave` | 波浪动作 | `serv wave 3 2` |
| `servo_test` | 自动测试 | `servo_test` |

### HMI显示命令
| 命令 | 功能 | 示例 |
|------|------|------|
| `hmi_test text` | 设置文本 | `hmi_test text t_msg "Hello"` |
| `hmi_test value` | 设置数值 | `hmi_test value n_servo1 2048` |
| `hmi_test wifi` | 测试WiFi显示 | `hmi_test wifi` |
| `hmi_test servo` | 测试舵机显示 | `hmi_test servo 1 2048` |

📖 **完整命令列表**: 详见 [命令参考手册.md](命令参考手册.md)

---

## 📁 项目结构

```
Cc/
├── applications/              # 应用层代码
│   ├── main.c                # 主程序
│   ├── wifi_manager.c/h      # WiFi管理模块
│   ├── servo_http_client.c/h # HTTP客户端
│   ├── servo_control.c/h     # 基础舵机控制
│   ├── servo_advanced.c/h    # 高级舵机控制
│   ├── servo_msh_advanced.c  # 舵机MSH命令
│   ├── hmi_display.c/h       # HMI显示驱动
│   └── hmi_callbacks.c       # HMI事件回调
│
├── board/                    # 板级支持包
│   ├── board.h               # 引脚定义
│   └── ...
│
├── rtconfig.h                # RT-Thread配置
├── USART HMI/                # 串口屏工程文件
├── 命令参考手册.md            # 命令文档
├── 开发日志.md               # 开发记录
└── README.md                 # 本文件
```

---

## 🛠️ 技术栈

### 操作系统
- **RT-Thread RTOS 5.1.0** - 实时操作系统内核
- **线程管理** - 多线程任务调度
- **设备驱动框架** - UART、Timer等

### 网络协议栈
- **lwIP** - 轻量级TCP/IP协议栈
- **CYW43438驱动** - WiFi芯片驱动
- **HTTP客户端** - 基于Socket实现

### 通信协议
- **SCSCL协议** - 舵机串行通信协议
- **TJC串口屏协议** - ASCII命令 + 帧尾

### 开发工具
- **RT-Thread Studio** / **MDK-ARM** - IDE
- **Git** - 版本控制
- **ST-Link** - 调试下载

---

## 📊 开发进度

### 里程碑

- ✅ **里程碑1**: WiFi舵机控制基础功能（2025-11-14完成）
- ✅ **里程碑2**: 串口屏显示系统状态（2025-11-15完成）
- 🎯 **里程碑3**: 肌电信号实时控制单手指
- 🎯 **里程碑4**: 四指独立控制
- 🎯 **里程碑5**: 完整系统演示

### 时间轴

```
┌──────────────────────────────────────────────────────┐
│                   项目时间轴                          │
├──────────────────────────────────────────────────────┤
│ 阶段1: WiFi舵机控制        │ ✅ 已完成 (2025-11-14)  │
│ 阶段2: 串口屏显示          │ ✅ 已完成 (2025-11-15)  │
│ 阶段3: 肌电信号控制        │ ⏳ 即将开始             │
│ 阶段4: 系统集成优化        │ ⏳                     │
│ 阶段5: 测试与完善          │ ⏳                     │
├──────────────────────────────────────────────────────┤
│ 预计总工期: 2-3周                                     │
└──────────────────────────────────────────────────────┘
```

---

## 💡 核心技术特点

### 1. 智能舵机切换
自动跟踪当前活动舵机ID，最小化HTTP请求次数，提高响应速度。

### 2. 线程安全设计
使用互斥锁保护共享资源，支持多线程并发调用。

### 3. 环形缓冲区
中断安全的数据缓冲机制，防止数据丢失，提高通信可靠性。

### 4. 事件驱动架构
基于信号量的异步事件处理，降低CPU占用率。

### 5. 模块化设计
驱动层、业务层分离，易于维护和扩展。

### 6. 电源管理经验
DC-DC降压方案，确保舵机稳定运行在额定电压8.4V。

---

## ⚠️ 重要提示

### 硬件安全

1. **电压匹配** ⚡
   - 舵机额定电压：8.4V (2S锂电池)
   - 必须使用DC-DC降压模块
   - 禁止直接12V供电！

2. **电流容量** 🔌
   - 4个舵机至少需要3A持续电流
   - 推荐5A峰值能力
   - 使用粗线供电（≥20AWG）

3. **共地连接** 🔗
   - 所有电源GND必须连接
   - 舵机电源端并联1000μF电容

4. **定期检查** 🔍
   - 测量电压（空载、带载）
   - 观察舵机温度（≤40℃）
   - 检查接线端子

### 软件使用

1. **WiFi优先** 📡
   - 所有舵机控制命令需先连接WiFi
   - 使用`connect_esp32`或`wifi_join`

2. **扭矩保护** 🛡️
   - 长时间不用建议关闭扭矩（`serv all_toff`）
   - 避免舵机过热和能耗

3. **紧急停止** 🛑
   - 遇到异常立即执行`serv all_stop`
   - 检查硬件连接和电源

---

## 🤝 贡献指南

欢迎贡献代码、报告问题或提出建议！

### 如何贡献

1. Fork本仓库
2. 创建您的特性分支 (`git checkout -b feature/AmazingFeature`)
3. 提交您的更改 (`git commit -m 'Add some AmazingFeature'`)
4. 推送到分支 (`git push origin feature/AmazingFeature`)
5. 打开Pull Request

### 代码规范

- 遵循RT-Thread代码风格
- 添加必要的注释和文档
- 确保代码通过编译
- 测试新功能

---

## 📄 许可证

本项目基于 **Apache-2.0** 许可证开源。

详见 [LICENSE](LICENSE) 文件。

---

## 👨‍💻 作者

**Cc**

- GitHub: [@ZenvaMea](https://github.com/ZenvaMea)
- 项目仓库: [RT_Thread-Program](https://github.com/ZenvaMea/RT_Thread-Program)

---

## 📞 技术支持

- **Issues**: [GitHub Issues](https://github.com/ZenvaMea/RT_Thread-Program/issues)
- **文档**:
  - [命令参考手册.md](命令参考手册.md)
  - [开发日志.md](开发日志.md)

---

## 🙏 致谢

- [RT-Thread](https://www.rt-thread.org/) - 优秀的国产RTOS
- [STMicroelectronics](https://www.st.com/) - STM32H7RS芯片
- [Espressif](https://www.espressif.com/) - ESP32 WiFi模块
- 所有为开源社区做出贡献的开发者

---

## 📈 项目统计

![GitHub stars](https://img.shields.io/github/stars/ZenvaMea/RT_Thread-Program?style=social)
![GitHub forks](https://img.shields.io/github/forks/ZenvaMea/RT_Thread-Program?style=social)
![GitHub issues](https://img.shields.io/github/issues/ZenvaMea/RT_Thread-Program)
![GitHub last commit](https://img.shields.io/github/last-commit/ZenvaMea/RT_Thread-Program)

---

<div align="center">

**⭐ 如果这个项目对您有帮助，请给它一个Star！⭐**

Made with ❤️ by Cc

© 2025 RT-Thread外骨骼手指控制系统

</div>
