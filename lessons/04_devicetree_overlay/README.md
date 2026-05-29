# Lesson 04: Devicetree Overlay

本课学习 Zephyr 应用级 devicetree overlay。示例不修改全局
`rocket_pi.dts`，只在本 lesson 的 `app.overlay` 中新增课程专用
`lesson-led`、`lesson-button` alias 和 `zephyr,lesson-*` chosen 节点。
应用代码通过这些名字选择蓝色 LED 和 PA0 用户按键。

## 参考资料

- Zephyr 官方文档：<https://docs.zephyrproject.org/latest/build/dts/index.html>
- Zephyr 官方文档：<https://docs.zephyrproject.org/latest/build/dts/howtos.html>
- Zephyr 官方 sample：`samples/basic/blinky`、`samples/basic/button`
- 开发板例程仓库：<https://github.com/newbie-jiang/stm32f401re_rocketpi>
- 对应 Rocket-Pi 文件：`rocket_pi.dts`
- 个人远程 GitHub 学习仓库：<https://github.com/Yang-1131501380/Zephyr>

## 本课目标

- 理解 board DTS 和应用 `app.overlay` 的职责边界。
- 使用 overlay 为 lesson 增加专用 alias 和 chosen 节点。
- 在 C 代码中使用 `DT_ALIAS()`、`DT_CHOSEN()` 和 `GPIO_DT_SPEC_GET()`。
- 构建后查看生成的 `zephyr.dts`，确认 overlay 已合并。
- 验证同一份应用逻辑可以通过 overlay 切换硬件节点。

## 硬件资源

| 功能 | Zephyr 节点或 alias | 引脚/外设 | 说明 |
| --- | --- | --- | --- |
| 本课 LED | `lesson-led` / `zephyr,lesson-led` | PB14 | overlay 指向 `blue_led`，低电平有效 |
| 本课按键 | `lesson-button` / `zephyr,lesson-button` | PA0 | overlay 指向 `user_button`，高电平有效 |
| 调试串口 | `zephyr,console` / `&usart2` | PA2 TX, PA3 RX | 115200 baud |

## 构建

在 west workspace 根目录 `/home/msy/zephyrproject` 下执行：

```sh
CCACHE_DISABLE=1 .venv/bin/west build -p always -b rocket_pi \
  apps/rocketpi_zephyr_learning/lessons/04_devicetree_overlay \
  -d build/rocketpi_04_devicetree_overlay
```

## 查看 overlay 合并结果

构建后检查生成的 devicetree：

```sh
grep -n "lesson-led\\|lesson-button" \
  build/rocketpi_04_devicetree_overlay/zephyr/zephyr.dts
```

也可以查看编译期生成的宏：

```sh
grep -n "DT_N_ALIAS_lesson" \
  build/rocketpi_04_devicetree_overlay/zephyr/include/generated/zephyr/devicetree_generated.h
```

## 烧录

优先尝试 OpenOCD：

```sh
.venv/bin/west flash -d build/rocketpi_04_devicetree_overlay -r openocd
```

如果当前虚拟机环境缺少 `/dev/bus/usb`，按 `docs/workflow.md` 记录的
ST-Link 虚拟盘方式烧录：

```sh
gio mount -d /dev/sdb
cp build/rocketpi_04_devicetree_overlay/zephyr/zephyr.hex \
  /run/media/msy/UNDEFINED/FLASH.HEX
sync
```

## 串口

```sh
picocom -b 115200 /dev/ttyACM0
```

## 预期现象

- 上电后串口打印课程名、本课 alias 解析到的 GPIO 端口、引脚和 label。
- 按下 PA0 用户按键：蓝色 LED 点亮。
- 松开 PA0 用户按键：蓝色 LED 熄灭。
- 串口只在轮询到按键状态变化时打印。

参考串口输出：

```text
Lesson 04: devicetree overlay
lesson-led: GPIOB pin 14, active low
lesson-button: GPIOA pin 0, active high
Overlay selected LED label: Lesson 04 overlay LED
Overlay selected button label: Lesson 04 overlay button
Button released -> overlay LED off
Button pressed -> overlay LED on
Button released -> overlay LED off
```

## 本课记录

- 当前固件产物构建后位于：

```text
build/rocketpi_04_devicetree_overlay/zephyr/zephyr.elf
build/rocketpi_04_devicetree_overlay/zephyr/zephyr.hex
build/rocketpi_04_devicetree_overlay/zephyr/zephyr.bin
```

- 本课 overlay 位于 `lessons/04_devicetree_overlay/app.overlay`。
- 应用代码没有直接使用 `led2` 或 `sw0`，而是使用本课自己的
  `lesson-led` 和 `lesson-button`。
- 2026-05-29 已构建通过，并确认 `app.overlay` 已合并进生成的
  `zephyr.dts`。
- 2026-05-29 已通过 OpenOCD 写入
  `build/rocketpi_04_devicetree_overlay/zephyr/zephyr.hex`。
- 2026-05-29 已通过 `/dev/ttyACM0` 串口验证启动日志，确认 overlay
  选中的 LED 为 `gpio@40020400 pin 14`，按键为 `gpio@40020000 pin 0`。

## 常见问题

- 构建时报 `lesson-led alias is missing`：确认 `app.overlay` 文件名在
  lesson 根目录，Zephyr 默认会自动合并它。
- 串口 label 不是本课文字：确认本课 build 目录已用 `-p always` 重新构建。
- LED 没反应：确认 overlay 指向的是 `blue_led`，蓝色 LED 为低电平有效，
  应用应通过 `gpio_pin_set_dt()` 写逻辑状态。
- 想改用红灯或绿灯：只改 `app.overlay` 中的 `&blue_led` 为 `&red_led`
  或 `&green_led`，应用 C 代码不需要改。

## 下一课

下一课建议做 `05_kconfig_prjconf`：学习 `prj.conf` 和 Kconfig 如何控制
编译选项、驱动依赖和功能裁剪。
