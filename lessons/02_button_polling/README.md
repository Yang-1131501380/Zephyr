# Lesson 02: Button Polling

本课用 Rocket-Pi 的 PA0 用户按键做 GPIO 输入轮询，并用红色 LED 显示
按键状态。按键按下时 LED 亮，松开时 LED 灭，串口只在状态变化时打印。

这是第一课 `01_blinky` 的自然延伸：第一课只做 GPIO 输出，本课加入
GPIO 输入，并开始理解按键有效电平、轮询周期和 Devicetree alias。

## 参考资料

- Zephyr 官方文档：<https://docs.zephyrproject.org/latest/>
- Zephyr 官方 sample：`samples/basic/button`
- Zephyr GPIO API：`include/zephyr/drivers/gpio.h`
- 开发板例程仓库：<https://github.com/newbie-jiang/stm32f401re_rocketpi>
- 对应 Rocket-Pi 例程：`rocketpi_key_scan`
- 个人远程 GitHub 学习仓库：<https://github.com/Yang-1131501380/Zephyr>

说明：Zephyr 当前官方 `samples/basic/button` 更偏向 input subsystem 的
事件回调写法。本课故意先使用 GPIO 轮询，因为它更适合第一次学习输入
引脚、上拉/下拉和有效电平。下一课再进入 GPIO interrupt。

## 本课目标

- 从 Devicetree alias `sw0` 获取用户按键。
- 从 Devicetree alias `led0` 获取红色 LED。
- 使用 `gpio_pin_configure_dt()` 配置输入和输出。
- 使用 `gpio_pin_get_dt()` 周期读取按键逻辑状态。
- 使用 `gpio_pin_set_dt()` 根据按键状态控制 LED。
- 通过串口只打印状态变化，避免刷屏。

## 硬件资源

| 功能 | Zephyr 节点或 alias | 引脚/外设 | 说明 |
| --- | --- | --- | --- |
| 用户按键 | `sw0` | PA0 | `GPIO_PULL_DOWN | GPIO_ACTIVE_HIGH` |
| 红色 LED | `led0` | PA1 | `GPIO_ACTIVE_LOW` |
| 调试串口 | `zephyr,console` / `&usart2` | PA2 TX, PA3 RX | 115200 baud |

## 文件结构

```text
lessons/02_button_polling/
  CMakeLists.txt
  prj.conf
  README.md
  study.md
  src/main.c
```

## 构建

在 west workspace 根目录 `/home/msy/zephyrproject` 下执行：

```sh
CCACHE_DISABLE=1 .venv/bin/west build -p always -b rocket_pi \
  apps/rocketpi_zephyr_learning/lessons/02_button_polling \
  -d build/rocketpi_02_button_polling
```

## 烧录

OpenOCD 烧录命令：

```sh
.venv/bin/west flash -d build/rocketpi_02_button_polling -r openocd
```

如果当前环境中 OpenOCD 因 `/dev/bus/usb` 缺失而无法打开 ST-Link，可使用
ST-Link 虚拟盘烧录：

```sh
gio mount -d /dev/sdb
cp build/rocketpi_02_button_polling/zephyr/zephyr.hex \
  /run/media/msy/UNDEFINED/FLASH.HEX
sync
```

注意：

- 虚拟盘一般显示为 `UNDEFINED`。
- 优先使用短文件名 `FLASH.HEX`，不要使用长文件名。
- 写入后 ST-Link 会自动烧录、复位并重新枚举。

## 串口

Rocket-Pi 控制台使用 USART2，波特率为 115200。

```sh
picocom -b 115200 /dev/ttyACM0
```

退出 `picocom`：先按 `Ctrl-A`，再按 `Ctrl-X`。

## 预期现象

- 上电后串口打印课程名、按键引脚、LED 引脚和轮询周期。
- 按下 PA0 用户按键：红色 LED 点亮。
- 松开 PA0 用户按键：红色 LED 熄灭。
- 串口只在按键状态变化时打印。

参考串口输出：

```text
Lesson 02: button polling
Button: GPIOA pin 0, active high
LED: GPIOA pin 1, active low
Polling interval: 20 ms
Button: released
Button: pressed
Button: released
```

## 本课记录

- 当前固件产物构建后位于：

```text
build/rocketpi_02_button_polling/zephyr/zephyr.elf
build/rocketpi_02_button_polling/zephyr/zephyr.hex
build/rocketpi_02_button_polling/zephyr/zephyr.bin
```

- Windows 工具烧录时优先使用 `zephyr.hex`。
- 如果使用 `zephyr.bin`，普通课程起始地址为 `0x08000000`。
- 2026-05-29 已通过 ST-Link/V2.1 虚拟盘方式写入
  `FLASH.HEX` 完成烧录。OpenOCD 在当前虚拟机环境中失败原因是缺少
  `/dev/bus/usb`，不是固件或 Rocket-Pi board 配置错误。

## 常见问题

- 构建提示找不到 `sw0`：检查 `boards/others/rocket_pi/rocket_pi.dts`
  是否定义了按键 alias。
- 按键逻辑反了：检查 `sw0` 的 `GPIO_ACTIVE_HIGH` 或 `GPIO_ACTIVE_LOW`。
- LED 逻辑反了：Rocket-Pi 红色 LED 是低电平有效，代码使用的是 Zephyr
  逻辑状态，不直接等同物理电平。
- 串口没有输出：确认串口号、115200 波特率、USB 连接和板子复位。

## 下一课

下一课建议做 `03_button_interrupt`：把本课的轮询改为 GPIO 中断，学习
`gpio_pin_interrupt_configure_dt()`、`gpio_init_callback()` 和按键消抖。
