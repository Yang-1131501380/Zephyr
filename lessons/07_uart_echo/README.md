# Lesson 07: UART Echo

本课使用 UART polling API 读取 console UART 输入，并把收到的字符原样回显。
Rocket-Pi 当前 console 是 USART2，波特率 115200。

## 重点

- `DEVICE_DT_GET(DT_CHOSEN(zephyr_console))`
- `uart_poll_in()`
- `uart_poll_out()`
- console UART 与应用主动读写 UART 的边界

## 构建

```sh
CCACHE_DISABLE=1 .venv/bin/west build -p always -b rocket_pi \
  apps/rocketpi_zephyr_learning/lessons/07_uart_echo \
  -d build/rocketpi_07_uart_echo
```

## 烧录和串口

```sh
.venv/bin/west flash -d build/rocketpi_07_uart_echo -r openocd
picocom -b 115200 /dev/ttyACM0
```

## 预期现象

- 串口打印课程名和 UART 设备名。
- 在串口终端输入字符，开发板立即回显。
- 回车会输出 CRLF，终端换行显示正常。

## 本课记录

- 构建产物：`build/rocketpi_07_uart_echo/zephyr/zephyr.hex`
- 本课使用 polling API，不使用中断或 DMA。
- 2026-05-29 已构建通过，待烧录和串口回显验证。
