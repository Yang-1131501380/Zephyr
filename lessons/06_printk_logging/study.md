# Lesson 06 Study Notes

本课建立后续调试输出的基础：`printk()` 适合最小、直接的串口输出；
logging 子系统适合可分模块、可分等级、可裁剪的运行日志。

## 学习目标

- 理解 `printk()` 与 logging 的职责差异。
- 掌握 `LOG_MODULE_REGISTER()` 的作用。
- 掌握 `LOG_ERR()`、`LOG_WRN()`、`LOG_INF()`、`LOG_DBG()` 的等级关系。
- 理解 `CONFIG_LOG`、`CONFIG_LOG_BACKEND_UART`、`CONFIG_LOG_DEFAULT_LEVEL`。
- 能通过 Kconfig 调整单个模块日志等级。

## printk

`printk()` 直接走 console 输出，使用简单：

```c
printk("Lesson 06: printk and logging\n");
```

它适合：

- 启动早期输出。
- 最小工程验证 console 是否可用。
- 临时打印少量信息。

它不适合：

- 大量模块化日志。
- 运行期按模块、按等级过滤。
- 需要统一格式、时间戳、后端的系统日志。

## Logging

logging 需要先注册模块：

```c
LOG_MODULE_REGISTER(lesson_06, CONFIG_LESSON_06_LOG_LEVEL);
```

然后使用不同等级：

```c
LOG_INF("Logging subsystem is running");
LOG_WRN("This warning is intentional");
LOG_DBG("Debug log is visible");
```

日志等级从高到低通常是：

```text
ERR -> WRN -> INF -> DBG
```

数值越高，输出越详细。本课把 `CONFIG_LESSON_06_LOG_LEVEL` 设为 `4`，
所以 debug 日志也会输出。

## prj.conf

本课关键配置：

```text
CONFIG_LOG=y
CONFIG_LOG_MODE_IMMEDIATE=y
CONFIG_LOG_BACKEND_UART=y
CONFIG_LOG_BACKEND_SHOW_COLOR=n
CONFIG_LOG_DEFAULT_LEVEL=3
CONFIG_LESSON_06_LOG_LEVEL=4
CONFIG_LESSON_06_HEARTBEAT_INTERVAL_MS=1000
```

说明：

- `CONFIG_LOG=y`：启用 logging 子系统。
- `CONFIG_LOG_MODE_IMMEDIATE=y`：日志调用点立即输出，学习阶段更直观。
- `CONFIG_LOG_BACKEND_UART=y`：把日志输出到 UART 后端。
- `CONFIG_LOG_BACKEND_SHOW_COLOR=n`：关闭颜色转义码，让串口日志更适合记录。
- `CONFIG_LOG_DEFAULT_LEVEL=3`：默认 info 级别。
- `CONFIG_LESSON_06_LOG_LEVEL=4`：本模块打开 debug。

## Kconfig

应用级 `Kconfig` 同样需要：

```text
source "Kconfig.zephyr"
```

然后定义本课自己的选项：

```text
config LESSON_06_LOG_LEVEL
	int "Lesson 06 module log level"
	default 4
	range 0 4
```

这样 C 代码中就可以用：

```c
LOG_MODULE_REGISTER(lesson_06, CONFIG_LESSON_06_LOG_LEVEL);
```

## 程序流程

1. 配置 `led1` 为输出。
2. 用 `printk()` 输出课程名和说明。
3. 用 `LOG_INF()` 打印 logging 已启动、配置值和 LED 引脚。
4. 用 `LOG_WRN()` 打印一条教学用 warning。
5. 用 `LOG_DBG()` 验证 debug 等级是否可见。
6. 主循环每秒翻转绿色 LED，并打印 heartbeat。

## 构建检查

```sh
grep -n "LOG\\|LESSON_06" build/rocketpi_06_printk_logging/zephyr/.config
```

如果看不到 `CONFIG_LOG=y`，说明 `prj.conf` 没有被合并或配置名写错。

## 常见错误

- `LOG_DBG()` 没输出：模块日志等级低于 debug，或全局日志等级裁剪了 debug。
- 日志格式与预期不同：不同 Zephyr 版本、后端和格式配置可能略有差异。
- 串口没有输出：先确认 `printk()` 是否有输出，再排查 logging 后端。
- 忘记 `LOG_MODULE_REGISTER()`：`LOG_*` 宏没有模块上下文。

## 课后练习

1. 把 `CONFIG_LESSON_06_LOG_LEVEL=3`，确认 debug 日志消失。
2. 把 `CONFIG_LESSON_06_HEARTBEAT_INTERVAL_MS=500`，观察心跳变快。
3. 注释 `CONFIG_LOG_BACKEND_UART=y`，观察 logging 输出变化。
4. 对比 `printk()` 和 `LOG_INF()` 输出格式差异。

## 下一课衔接

下一课 `07_uart_echo` 会从“通过 console 看日志”进入“应用主动读写 UART”，
重点是 UART polling API、收发超时和 console UART 的边界。
