# Lesson 06: printk and Logging

本课从简单 `printk()` 过渡到 Zephyr logging 子系统。示例使用绿色 LED
作为心跳，每秒切换一次，同时通过串口输出 `printk()`、`LOG_INF()`、
`LOG_WRN()` 和 `LOG_DBG()`，观察日志等级、模块名和输出格式。

## 参考资料

- Zephyr logging 文档：<https://docs.zephyrproject.org/latest/services/logging/index.html>
- Zephyr 官方 sample：`samples/subsys/logging/logger`
- Zephyr 官方 sample：`samples/basic/blinky`
- 开发板例程仓库：<https://github.com/newbie-jiang/stm32f401re_rocketpi>
- 对应 Rocket-Pi 参考：`rocketpi_uart_printf`
- 个人远程 GitHub 学习仓库：<https://github.com/Yang-1131501380/Zephyr>

## 本课目标

- 区分 `printk()` 和 Zephyr logging。
- 使用 `LOG_MODULE_REGISTER()` 注册日志模块。
- 使用 `LOG_INF()`、`LOG_WRN()`、`LOG_DBG()` 输出不同等级日志。
- 通过 `prj.conf` 打开 `CONFIG_LOG` 和 UART 日志后端。
- 用 lesson 自定义 Kconfig 控制模块日志等级和心跳周期。

## 硬件资源

| 功能 | Zephyr 节点或 alias | 引脚/外设 | 说明 |
| --- | --- | --- | --- |
| 绿色 LED | `led1` | PB10 | 低电平有效，每秒心跳翻转 |
| 调试串口 | `zephyr,console` / `&usart2` | PA2 TX, PA3 RX | 115200 baud |

## 构建

在 west workspace 根目录 `/home/msy/zephyrproject` 下执行：

```sh
CCACHE_DISABLE=1 .venv/bin/west build -p always -b rocket_pi \
  apps/rocketpi_zephyr_learning/lessons/06_printk_logging \
  -d build/rocketpi_06_printk_logging
```

## 查看配置结果

```sh
grep -n "LOG\\|LESSON_06" build/rocketpi_06_printk_logging/zephyr/.config
```

重点确认：

```text
CONFIG_LOG=y
CONFIG_LOG_MODE_IMMEDIATE=y
CONFIG_LOG_BACKEND_UART=y
CONFIG_LOG_BACKEND_SHOW_COLOR=n
CONFIG_LESSON_06_LOG_LEVEL=4
CONFIG_LESSON_06_HEARTBEAT_INTERVAL_MS=1000
```

## 烧录

```sh
.venv/bin/west flash -d build/rocketpi_06_printk_logging -r openocd
```

## 串口

```sh
picocom -b 115200 /dev/ttyACM0
```

## 预期现象

- 上电后串口先看到 Zephyr boot banner。
- `printk()` 输出两行普通文本。
- logging 输出带日志等级、模块名和消息内容。
- 绿色 LED 每秒翻转一次。
- 每 5 次心跳输出一条 warning。

参考串口输出：

```text
Lesson 06: printk and logging
printk() writes directly to the console path.
[00:00:00.000,000] <inf> lesson_06: Logging subsystem is running
[00:00:00.000,000] <wrn> lesson_06: This warning is intentional for lesson output
[00:00:00.000,000] <dbg> lesson_06: Debug log is visible because LESSON_06_LOG_LEVEL=4
[00:00:01.000,000] <inf> lesson_06: Heartbeat 1: green LED on
```

## 本课记录

- 当前固件产物构建后位于：

```text
build/rocketpi_06_printk_logging/zephyr/zephyr.elf
build/rocketpi_06_printk_logging/zephyr/zephyr.hex
build/rocketpi_06_printk_logging/zephyr/zephyr.bin
```

- 本课新增应用级 `Kconfig`，先 `source "Kconfig.zephyr"`，再定义
  `CONFIG_LESSON_06_*`。
- `CONFIG_LOG_MODE_IMMEDIATE=y` 让日志立即输出，方便学习阶段串口观察。
- `CONFIG_LOG_BACKEND_SHOW_COLOR=n` 关闭 ANSI 颜色，串口记录更容易阅读。
- 2026-05-29 已构建通过，并确认 `.config` 中启用了 UART logging backend。
- 2026-05-29 已通过 OpenOCD 写入
  `build/rocketpi_06_printk_logging/zephyr/zephyr.hex`。
- 2026-05-29 已通过 `/dev/ttyACM0` 串口验证 `printk()`、info、warning、
  debug 和 heartbeat 输出。

## 常见问题

- 没有 logging 输出：确认 `CONFIG_LOG=y` 和 `CONFIG_LOG_BACKEND_UART=y`。
- 看不到 debug 日志：确认 `CONFIG_LESSON_06_LOG_LEVEL=4`，并重新 pristine 构建。
- 只有 `printk()` 没有 `LOG_*`：通常是 logging 后端或日志等级配置问题。
- 日志太多：把 `CONFIG_LESSON_06_LOG_LEVEL` 改成 `3`，只保留 info 及以上。

## 下一课

下一课建议做 `07_uart_echo`：学习 UART polling 收发，区分 console UART 和
应用主动读写的 UART。
