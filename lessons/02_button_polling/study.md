# Lesson 02 Study Notes

本课重点是把第一课的 GPIO 输出扩展到 GPIO 输入。不要急着上中断，先把
最基础的输入配置、有效电平和轮询读值弄清楚。

## 学习目标

- 理解按键输入和 LED 输出在 Zephyr 中都属于 GPIO 设备。
- 理解应用代码为什么使用 `sw0`、`led0`，而不是直接写 PA0、PA1。
- 理解 `GPIO_ACTIVE_HIGH`、`GPIO_ACTIVE_LOW` 表示的是逻辑有效状态。
- 掌握 `gpio_dt_spec` 和 `_dt` 系列 GPIO API 的基本用法。
- 知道轮询方案的优点、缺点和适用范围。

## 官方 sample

先阅读 Zephyr 官方 `samples/basic/button`。

需要注意：官方 sample 可能使用 input subsystem 来接收按键事件，本课使用
GPIO 轮询，原因是学习路径不同：

- 官方 sample 更接近 Zephyr 较新的输入事件模型。
- 本课更适合初学者理解“一个 GPIO 输入脚如何被读取”。
- 后续课程会逐步过渡到 GPIO interrupt、消息队列和 input subsystem。

## Rocket-Pi 对照例程

- 开发板例程仓库：<https://github.com/newbie-jiang/stm32f401re_rocketpi>
- 对应例程：`rocketpi_key_scan`

建议对照时只看三件事：

1. Rocket-Pi 用户按键接在哪个引脚。
2. 裸机或 RTOS 示例如何判断按下和松开。
3. Zephyr 版本如何把引脚和有效电平移动到 Devicetree 中。

## Devicetree

Rocket-Pi 当前按键定义在 `boards/others/rocket_pi/rocket_pi.dts`：

```dts
gpio_keys {
	compatible = "gpio-keys";

	user_button: button {
		label = "User";
		gpios = <&gpioa 0 (GPIO_PULL_DOWN | GPIO_ACTIVE_HIGH)>;
		zephyr,code = <INPUT_KEY_0>;
	};
};

aliases {
	sw0 = &user_button;
	led0 = &red_led;
};
```

代码中通过 alias 获取硬件：

```c
#define BUTTON_NODE DT_ALIAS(sw0)
#define LED_NODE DT_ALIAS(led0)

static const struct gpio_dt_spec button =
	GPIO_DT_SPEC_GET(BUTTON_NODE, gpios);
static const struct gpio_dt_spec led =
	GPIO_DT_SPEC_GET(LED_NODE, gpios);
```

这样做的意义：

- 应用代码不直接绑定 PA0 和 PA1。
- 换板或换引脚时，优先修改 DTS，不改业务逻辑。
- `gpio_dt_spec` 同时保存 GPIO 控制器、pin 编号和有效电平 flags。

构建后可以查看最终生成的 Devicetree：

```text
build/rocketpi_02_button_polling/zephyr/zephyr.dts
```

重点搜索：

```text
sw0
led0
gpioa
button
led_0
```

## prj.conf

本课配置：

```text
CONFIG_GPIO=y
CONFIG_SERIAL=y
CONFIG_CONSOLE=y
CONFIG_UART_CONSOLE=y
CONFIG_PRINTK=y
```

含义：

- `CONFIG_GPIO=y`：启用 GPIO driver API。
- `CONFIG_SERIAL=y`：启用串口驱动。
- `CONFIG_CONSOLE=y`：启用 console 子系统。
- `CONFIG_UART_CONSOLE=y`：把 console 绑定到 UART。
- `CONFIG_PRINTK=y`：允许使用 `printk()` 输出调试信息。

如果去掉 `CONFIG_GPIO=y`，GPIO API 可能无法正常链接。  
如果去掉串口和 console 相关配置，程序仍可能运行，但你看不到串口日志。

## 核心 API

### `gpio_is_ready_dt()`

检查 Devicetree 中描述的 GPIO 控制器是否已经初始化完成。

本课分别检查按键和 LED：

```c
if (!gpio_is_ready_dt(&button)) {
	printk("Button GPIO is not ready\n");
	return -ENODEV;
}
```

如果不检查，后面直接配置 pin，出错时定位会更困难。

### `gpio_pin_configure_dt()`

配置 GPIO 方向和初始状态。

```c
gpio_pin_configure_dt(&button, GPIO_INPUT);
gpio_pin_configure_dt(&led, GPIO_OUTPUT_INACTIVE);
```

按键配置为输入。LED 配置为输出且初始为 inactive，避免上电后 LED 状态
不确定。

### `gpio_pin_get_dt()`

读取 GPIO 逻辑状态：

```c
pressed = gpio_pin_get_dt(&button);
```

返回值含义：

- `< 0`：读取失败。
- `0`：逻辑无效状态，本课表示松开。
- `1`：逻辑有效状态，本课表示按下。

### `gpio_pin_set_dt()`

设置 GPIO 逻辑状态：

```c
gpio_pin_set_dt(&led, is_pressed ? 1 : 0);
```

注意：Rocket-Pi 红色 LED 是 `GPIO_ACTIVE_LOW`，所以这里传入 `1` 表示
“让 LED 进入有效状态”，不代表物理输出高电平。

### `k_msleep()`

让当前线程睡眠一段时间：

```c
k_msleep(BUTTON_POLL_INTERVAL_MS);
```

本课轮询周期是 20 ms。这个值足够快，按键响应不会明显迟钝；同时又
不会让主循环无意义地高速占用 CPU。

## 程序流程

1. 编译期确认 `sw0` 和 `led0` alias 存在。
2. 从 Devicetree 生成 `button` 和 `led` 两个 `gpio_dt_spec`。
3. 检查 GPIO 控制器是否 ready。
4. 配置按键为输入。
5. 配置 LED 为输出 inactive。
6. 打印课程名、引脚和轮询周期。
7. 循环读取按键状态。
8. 如果状态变化，串口打印 `pressed` 或 `released`。
9. 用按键状态同步控制红色 LED。
10. 睡眠 20 ms 后再次读取。

## 为什么本课不用中断

轮询的优点：

- 程序结构最简单。
- 容易用串口观察每一步。
- 适合理解输入引脚和有效电平。

轮询的缺点：

- 响应速度受轮询周期影响。
- 周期太短会浪费 CPU。
- 周期太长会漏掉很短的按键动作。
- 真实产品里通常还要做消抖。

所以本课先用轮询建立基础。下一课再用 GPIO interrupt 解决“持续检查”
的问题。

## 常见问题

### 构建失败：`sw0` 或 `led0` 不存在

检查 `rocket_pi.dts` 的 `aliases`：

```dts
aliases {
	sw0 = &user_button;
	led0 = &red_led;
};
```

### 按下后 LED 不亮

按顺序检查：

1. 串口是否打印 `Button: pressed`。
2. 如果串口有 pressed，说明按键输入正常，再查 LED。
3. 如果串口没有 pressed，检查 PA0 按键和 `GPIO_ACTIVE_HIGH`。
4. 检查 LED 是否接在 `led0`，以及是否为 `GPIO_ACTIVE_LOW`。

### 一直显示 pressed

可能原因：

- PA0 外部电路或按键状态异常。
- DTS 中上拉/下拉配置不匹配实际硬件。
- 按键被按住或短接。

### 串口刷屏

本课只在状态变化时打印。如果出现持续刷屏，说明读取值在 0 和 1 之间
快速抖动。可以先检查硬件按键，再考虑软件消抖。消抖会放到下一课中断
课程继续学习。

## 课后练习

1. 把红色 LED 改成绿色 LED：把代码中的 `led0` 改成 `led1`。
2. 把轮询周期从 20 ms 改成 200 ms，观察按键响应变慢。
3. 改成按下显示红色，松开显示蓝色。
4. 增加一个按下计数，只在 released 到 pressed 的边沿加一。
5. 查看 `build/rocketpi_02_button_polling/zephyr/zephyr.dts`，找出 PA0
   和 PA1 的最终配置。

## 下一课衔接

下一课 `03_button_interrupt` 建议解决三个问题：

- 不再每 20 ms 主动读取按键，而是在电平变化时触发中断。
- 在中断回调里只做最小工作，不做复杂业务。
- 加入最简单的软件消抖，理解机械按键为什么会抖动。
