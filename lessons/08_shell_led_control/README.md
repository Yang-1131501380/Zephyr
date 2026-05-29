# Lesson 08: Shell LED Control

本课启用 Zephyr Shell，在串口中注册 `led` 命令控制 Rocket-Pi RGB LED。

## 重点

- `CONFIG_SHELL`
- `CONFIG_SHELL_BACKEND_SERIAL`
- `SHELL_CMD_ARG_REGISTER()`
- shell 命令参数解析

## 构建

```sh
CCACHE_DISABLE=1 .venv/bin/west build -p always -b rocket_pi \
  apps/rocketpi_zephyr_learning/lessons/08_shell_led_control \
  -d build/rocketpi_08_shell_led_control
```

## 运行命令

```text
led red on
led green on
led blue off
led all off
```

## 本课记录

- 构建产物：`build/rocketpi_08_shell_led_control/zephyr/zephyr.hex`
- RGB LED 均为低电平有效，代码使用 `gpio_pin_set_dt()` 写逻辑状态。
- 2026-05-29 已构建通过，待烧录和 shell 命令验证。
