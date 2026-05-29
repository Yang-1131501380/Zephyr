# Lesson 05: Kconfig and prj.conf

本课学习 Zephyr 的 Kconfig 和 `prj.conf`。示例定义了本 lesson 自己的
`CONFIG_LESSON_05_*` 选项，并在 `prj.conf` 中设置它们。应用代码根据
这些配置决定 LED 闪烁周期、是否需要按住 PA0 才闪烁、是否打印计数日志。

## 参考资料

- Zephyr Kconfig 文档：<https://docs.zephyrproject.org/latest/build/kconfig/index.html>
- Zephyr 配置系统：<https://docs.zephyrproject.org/latest/build/kconfig/setting.html>
- Zephyr 官方 samples：`samples/basic/blinky`、`samples/basic/button`
- 开发板例程仓库：<https://github.com/newbie-jiang/stm32f401re_rocketpi>
- 个人远程 GitHub 学习仓库：<https://github.com/Yang-1131501380/Zephyr>

## 本课目标

- 理解 `prj.conf` 只设置配置值，真正的配置项需要由 Kconfig 定义。
- 定义 lesson 自己的 `Kconfig` 选项。
- 在 C 代码中使用 `CONFIG_*` 和 `IS_ENABLED()`。
- 理解 `bool`、`int`、`range`、`default`、`help` 的基本作用。
- 构建后查看 `.config` 和 `autoconf.h`，确认配置如何进入编译。

## 硬件资源

| 功能 | Zephyr 节点或 alias | 引脚/外设 | 说明 |
| --- | --- | --- | --- |
| 红色 LED | `led0` | PA1 | 低电平有效 |
| 用户按键 | `sw0` | PA0 | 高电平有效，启用 button gate 时使用 |
| 调试串口 | `zephyr,console` / `&usart2` | PA2 TX, PA3 RX | 115200 baud |

## 构建

在 west workspace 根目录 `/home/msy/zephyrproject` 下执行：

```sh
CCACHE_DISABLE=1 .venv/bin/west build -p always -b rocket_pi \
  apps/rocketpi_zephyr_learning/lessons/05_kconfig_prjconf \
  -d build/rocketpi_05_kconfig_prjconf
```

## 查看配置结果

构建后检查最终 `.config`：

```sh
grep -n "LESSON_05" build/rocketpi_05_kconfig_prjconf/zephyr/.config
```

也可以查看 C 代码实际包含的宏：

```sh
grep -n "LESSON_05" \
  build/rocketpi_05_kconfig_prjconf/zephyr/include/generated/zephyr/autoconf.h
```

## 烧录

```sh
.venv/bin/west flash -d build/rocketpi_05_kconfig_prjconf -r openocd
```

如果 OpenOCD 在当前虚拟机环境打不开 ST-Link，可按 `docs/workflow.md`
记录的 ST-Link 虚拟盘方式烧录 `zephyr.hex`。

## 串口

```sh
picocom -b 115200 /dev/ttyACM0
```

## 预期现象

- 上电后串口打印课程名和当前 `CONFIG_LESSON_05_*` 配置值。
- 默认 `CONFIG_LESSON_05_USE_BUTTON_GATE=y`，需要按住 PA0，红色 LED 才会
  按 `CONFIG_LESSON_05_BLINK_INTERVAL_MS` 周期闪烁。
- 默认 `CONFIG_LESSON_05_PRINT_COUNTER=y`，每次 LED 状态切换都会打印计数。

参考串口输出：

```text
Lesson 05: Kconfig and prj.conf
Blink interval: 300 ms
Button gate: enabled
Counter log: enabled
LED: gpio@40020000 pin 1, active low
Hold the user button to allow LED blinking.
Blink 1: LED on
Blink 2: LED off
```

## 本课记录

- 当前固件产物构建后位于：

```text
build/rocketpi_05_kconfig_prjconf/zephyr/zephyr.elf
build/rocketpi_05_kconfig_prjconf/zephyr/zephyr.hex
build/rocketpi_05_kconfig_prjconf/zephyr/zephyr.bin
```

- 本课新增应用级 `Kconfig`，配置项前缀统一为 `LESSON_05_`。
- 应用级 `Kconfig` 需要先 `source "Kconfig.zephyr"`，再定义 lesson 自己的
  配置项，否则 Zephyr 的 `GPIO`、`SERIAL` 等基础符号不会被加载。
- 修改 `prj.conf` 后需要重新构建，建议使用 `west build -p always`。
- 2026-05-29 已构建通过，`.config` 和 `autoconf.h` 均生成
  `CONFIG_LESSON_05_*`。
- 2026-05-29 已通过 OpenOCD 写入
  `build/rocketpi_05_kconfig_prjconf/zephyr/zephyr.hex`。
- 2026-05-29 已通过 `/dev/ttyACM0` 串口验证启动日志，确认周期为 300 ms，
  button gate 和 counter log 均为 enabled。

## 常见问题

- `prj.conf` 里写了 `CONFIG_LESSON_05_*` 但构建报 unknown symbol：检查
  lesson 根目录是否存在 `Kconfig`，配置名是否一致。
- 修改了 `CONFIG_LESSON_05_BLINK_INTERVAL_MS` 但现象没变：确认重新 pristine
  构建，并检查 `.config` 中最终值。
- `CONFIG_LESSON_05_USE_BUTTON_GATE=n` 后还想使用按键：代码里对应路径会被
  `IS_ENABLED()` 编译裁剪，需要重新打开该配置。

## 下一课

下一课建议做 `06_printk_logging`：在 `printk()` 基础上学习 Zephyr logging
子系统、日志等级和模块化日志输出。
