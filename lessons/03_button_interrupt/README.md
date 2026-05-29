# Lesson 03: Button Interrupt

本课把第二课的按键轮询改成 GPIO 中断。PA0 用户按键发生按下或松开时，
中断回调只触发一个延时 work；延时 30 ms 后再读取稳定电平，用红色 LED
显示按键状态，并通过串口打印状态变化。

## 参考资料

- Zephyr 官方文档：<https://docs.zephyrproject.org/latest/>
- Zephyr 官方 sample：`samples/basic/button`
- Zephyr GPIO API：`include/zephyr/drivers/gpio.h`
- 开发板例程仓库：<https://github.com/newbie-jiang/stm32f401re_rocketpi>
- 对应 Rocket-Pi 例程：`rocketpi_key_irq`
- 个人远程 GitHub 学习仓库：<https://github.com/Yang-1131501380/Zephyr>

## 本课目标

- 使用 `gpio_pin_interrupt_configure_dt()` 启用 GPIO 双边沿中断。
- 使用 `gpio_init_callback()` 和 `gpio_add_callback()` 注册回调。
- 理解 GPIO 回调运行在中断上下文，不在回调里做耗时工作。
- 使用 `k_work_delayable` 做最小软件消抖。
- 在 workqueue 上下文中读取稳定按键状态、控制 LED、打印串口。

## 硬件资源

| 功能 | Zephyr 节点或 alias | 引脚/外设 | 说明 |
| --- | --- | --- | --- |
| 用户按键 | `sw0` | PA0 | `GPIO_PULL_DOWN | GPIO_ACTIVE_HIGH` |
| 红色 LED | `led0` | PA1 | `GPIO_ACTIVE_LOW` |
| 调试串口 | `zephyr,console` / `&usart2` | PA2 TX, PA3 RX | 115200 baud |

## 构建

在 west workspace 根目录 `/home/msy/zephyrproject` 下执行：

```sh
CCACHE_DISABLE=1 .venv/bin/west build -p always -b rocket_pi \
  apps/rocketpi_zephyr_learning/lessons/03_button_interrupt \
  -d build/rocketpi_03_button_interrupt
```

## 烧录

优先尝试 OpenOCD：

```sh
.venv/bin/west flash -d build/rocketpi_03_button_interrupt -r openocd
```

如果当前虚拟机环境缺少 `/dev/bus/usb`，按 `docs/workflow.md` 记录的
ST-Link 虚拟盘方式烧录：

```sh
gio mount -d /dev/sdb
cp build/rocketpi_03_button_interrupt/zephyr/zephyr.hex \
  /run/media/msy/UNDEFINED/FLASH.HEX
sync
```

## 串口

```sh
picocom -b 115200 /dev/ttyACM0
```

## 预期现象

- 上电后串口打印课程名、按键引脚、LED 引脚和消抖时间。
- 按下 PA0 用户按键：红色 LED 点亮。
- 松开 PA0 用户按键：红色 LED 熄灭。
- 串口只在按键稳定状态变化后打印。

参考串口输出：

```text
Lesson 03: button interrupt
Button: GPIOA pin 0, active high
LED: GPIOA pin 1, active low
Debounce delay: 30 ms
Button: released
Button: pressed
Button: released
```

## 本课记录

- 当前固件产物构建后位于：

```text
build/rocketpi_03_button_interrupt/zephyr/zephyr.elf
build/rocketpi_03_button_interrupt/zephyr/zephyr.hex
build/rocketpi_03_button_interrupt/zephyr/zephyr.bin
```

- 本课使用中断加延时 work 消抖，不再使用 20 ms 主循环轮询。
- 中断回调中不直接读按键、不控制 LED、不打印串口。
- 2026-05-29 已通过 ST-Link/V2.1 虚拟盘方式写入
  `FLASH.HEX` 完成烧录，等待串口和按键现象验证。

## 常见问题

- 按键没有反应：确认 `sw0` alias 指向 PA0，且中断配置返回值正常。
- 串口打印多次：机械按键可能抖动，尝试把消抖时间从 30 ms 调到 50 ms。
- LED 逻辑反了：Rocket-Pi 红色 LED 是低电平有效，代码使用 Zephyr
  逻辑状态控制。
- OpenOCD 烧录失败：先读 `docs/workflow.md` 中的 ST-Link 虚拟盘烧录记录。

## 下一课

下一课建议做 `04_devicetree_overlay`：学习如何在 lesson 中用 overlay
覆盖或扩展硬件描述，而不是每次都直接修改 board 主 DTS。
