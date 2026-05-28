# Lesson 01: Blinky

本课使用 Rocket-Pi 板载 RGB LED 完成 Zephyr 的第一个 GPIO 示例。
程序会依次点亮红、绿、蓝三个 LED，并通过串口打印当前 LED 名称。

## 示例目标

- 使用自定义 `rocket_pi` board 构建 Zephyr 应用。
- 从 Devicetree alias 获取 LED GPIO。
- 使用 Zephyr GPIO Driver API 控制引脚输出。
- 使用 `printk()` 通过串口观察程序运行状态。

## 构建

在 west workspace 根目录 `/home/msy/zephyrproject` 下执行：

```sh
.venv/bin/west build -p always -b rocket_pi \
  apps/rocketpi_zephyr_learning/lessons/01_blinky \
  -d build/rocketpi_01_blinky
```

## 烧录

```sh
.venv/bin/west flash -d build/rocketpi_01_blinky -r openocd
```

## 串口

Rocket-Pi 控制台使用 USART2，波特率为 115200。

```sh
picocom -b 115200 /dev/ttyACM0
```

## 预期现象

- 红色 LED 点亮约 100 ms 后熄灭。
- 绿色 LED 点亮约 100 ms 后熄灭。
- 蓝色 LED 点亮约 100 ms 后熄灭。
- 串口循环输出：

```text
LED: red
LED: green
LED: blue
```

## 本课记录

- 已在 Rocket-Pi 上完成构建和烧录验证。
- 当前固件产物位于 west workspace 的
  `build/rocketpi_01_blinky/zephyr/zephyr.hex` 和
  `build/rocketpi_01_blinky/zephyr/zephyr.bin`。
- Windows 工具烧录时优先使用 `zephyr.hex`；如果使用 `zephyr.bin`，起始地址为
  `0x08000000`。
- 串口验证输出为 `LED: red`、`LED: green`、`LED: blue` 循环打印。
