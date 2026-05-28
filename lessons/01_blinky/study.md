# Lesson 01 Study Notes

本课重点不是“让灯闪起来”，而是建立 Zephyr 的基本工程模型。

## 需要理解的文件

- `CMakeLists.txt`：声明这是一个 Zephyr 应用，并指定 `src/main.c`。
- `prj.conf`：打开本课需要的 Zephyr 功能，例如 GPIO 和串口打印。
- `src/main.c`：应用逻辑，负责配置 LED 并循环控制亮灭。
- `boards/others/rocket_pi/rocket_pi.dts`：描述 Rocket-Pi 的硬件资源。

## Zephyr 应用结构

Zephyr 应用通常至少包含：

```text
CMakeLists.txt
prj.conf
src/main.c
```

本仓库的板级文件放在课程目录的上两级，因此课程 `CMakeLists.txt` 中使用
`BOARD_ROOT` 告诉 Zephyr 去哪里查找 `rocket_pi` 这块板。

## prj.conf 和 Kconfig

`prj.conf` 用来选择 Zephyr 功能。本课使用：

```text
CONFIG_GPIO=y
CONFIG_SERIAL=y
CONFIG_CONSOLE=y
CONFIG_UART_CONSOLE=y
CONFIG_PRINTK=y
```

如果没有 `CONFIG_GPIO=y`，GPIO 驱动相关 API 可能不会被启用。
如果没有串口和 console 配置，`printk()` 不一定能从串口输出。

## Devicetree 和 alias

代码中没有直接写 `PA1`、`PB10`、`PB14`，而是使用：

```c
GPIO_DT_SPEC_GET(DT_ALIAS(led0), gpios)
```

这表示代码通过 Devicetree alias 查找硬件。对应关系在
`rocket_pi.dts` 中：

```dts
aliases {
	led0 = &red_led;
	led1 = &green_led;
	led2 = &blue_led;
};
```

这样做的好处是：硬件连接变化时，优先修改 Devicetree，应用代码可以尽量
保持不变。

## GPIO API

本课用到三个核心 API：

- `gpio_is_ready_dt()`：确认 GPIO 控制器已经可用。
- `gpio_pin_configure_dt()`：把 LED 引脚配置为输出。
- `gpio_pin_set_dt()`：设置输出电平，控制 LED 亮灭。

`gpio_dt_spec` 是从 Devicetree 生成的 GPIO 描述，里面包含 GPIO 控制器、
引脚编号和有效电平等信息。

## 常见问题

- 构建提示找不到 `rocket_pi`：检查 `BOARD_ROOT` 是否指向仓库根目录。
- 串口无输出：确认串口号、115200 波特率、USB 连接和板子复位。
- LED 不亮：确认 `rocket_pi.dts` 中 LED 引脚和硬件原理图一致。
- 亮灭逻辑反了：检查 LED 是否为 `GPIO_ACTIVE_LOW`。

## 课后练习

1. 把 `LED_ON_TIME_MS` 从 500 改成 100，观察闪烁速度变化。
2. 修改 `led_names` 的顺序，并观察串口输出是否变化。
3. 修改 `while` 循环中的顺序，让 LED 按蓝、绿、红闪烁。
