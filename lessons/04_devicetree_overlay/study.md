# Lesson 04 Study Notes

本课重点是把硬件描述和应用逻辑分开：board DTS 描述开发板真实硬件，
lesson 的 `app.overlay` 描述本应用希望如何使用这些硬件。

## 学习目标

- 理解 Zephyr devicetree 的来源：SoC `.dtsi`、board `.dts`、应用 overlay。
- 掌握 `app.overlay` 的自动发现和合并规则。
- 掌握 alias、chosen 和节点 label 的区别。
- 能在 C 代码中通过 devicetree 宏获取 GPIO 配置。
- 能在构建产物中检查最终生成的 `zephyr.dts`。

## 本课 overlay

`app.overlay` 中没有新增真实硬件，只给已有硬件增加本课自己的名字：

```dts
/ {
	chosen {
		zephyr,lesson-led = &blue_led;
		zephyr,lesson-button = &user_button;
	};

	aliases {
		lesson-led = &blue_led;
		lesson-button = &user_button;
	};
};
```

这表示本课把 `lesson-led` 解析到 board DTS 中已有的 `blue_led`，
把 `lesson-button` 解析到已有的 `user_button`。如果以后想把本课 LED
换成红灯，只改 overlay：

```dts
lesson-led = &red_led;
```

应用代码仍然使用 `DT_ALIAS(lesson_led)`。

## alias、chosen、label

### 节点 label

board DTS 中的：

```dts
blue_led: led_2 {
	gpios = <&gpiob 14 GPIO_ACTIVE_LOW>;
	label = "User LD2";
};
```

其中 `blue_led:` 是节点 label。overlay 可以用 `&blue_led` 引用这个节点。

### alias

alias 是应用友好的短名字：

```dts
aliases {
	lesson-led = &blue_led;
};
```

C 代码中 `lesson-led` 会写成下划线形式：

```c
#define LED_NODE DT_ALIAS(lesson_led)
```

### chosen

chosen 通常用于“系统选定资源”，例如 console、flash、display。本课增加：

```dts
chosen {
	zephyr,lesson-led = &blue_led;
};
```

C 代码中读取：

```c
#define CHOSEN_LED_NODE DT_CHOSEN(zephyr_lesson_led)
```

本课同时放 alias 和 chosen，是为了练习两种写法。普通应用里通常选一种
清晰的命名方式即可。

## C 代码中的 devicetree 宏

本课核心代码：

```c
#define LED_NODE DT_ALIAS(lesson_led)

static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED_NODE, gpios);
```

`GPIO_DT_SPEC_GET()` 会在编译期生成：

```text
led.port     = GPIOB device
led.pin      = 14
led.dt_flags = GPIO_ACTIVE_LOW
```

所以应用不需要写 `GPIOB`、`14` 或有效电平。硬件变更优先放在 DTS/overlay。

## prj.conf

```text
CONFIG_GPIO=y
CONFIG_SERIAL=y
CONFIG_CONSOLE=y
CONFIG_UART_CONSOLE=y
CONFIG_PRINTK=y
```

说明：

- `CONFIG_GPIO=y`：启用 GPIO 驱动 API。
- `CONFIG_SERIAL=y`：启用串口驱动。
- `CONFIG_CONSOLE=y` 和 `CONFIG_UART_CONSOLE=y`：把 console 绑定到
  board DTS 的 `zephyr,console`。
- `CONFIG_PRINTK=y`：允许使用 `printk()` 输出学习日志。

## 构建产物检查

构建后最重要的检查文件是：

```text
build/rocketpi_04_devicetree_overlay/zephyr/zephyr.dts
```

它是最终合并后的 devicetree。确认以下内容：

```dts
chosen {
	zephyr,lesson-led = &blue_led;
	zephyr,lesson-button = &user_button;
};

aliases {
	lesson-led = &blue_led;
	lesson-button = &user_button;
};
```

如果这里没有本课 alias，说明 overlay 没被合并，先检查文件名是否为
`app.overlay`，路径是否在 lesson 根目录。

## 程序流程

1. 编译期通过 `DT_ALIAS(lesson_led)` 和 `DT_ALIAS(lesson_button)` 找到节点。
2. 编译期通过 `GPIO_DT_SPEC_GET()` 生成 GPIO 端口、引脚和 flag。
3. 启动后配置 LED 为输出、按键为输入。
4. 主循环每 20 ms 读取按键逻辑状态。
5. 按键按下时点亮 overlay 选中的 LED，松开时熄灭。

## 常见错误

- 在 C 代码里写 `DT_ALIAS(lesson-led)`：错误。C 宏中要用
  `DT_ALIAS(lesson_led)`。
- 修改 overlay 后没有变化：旧 build 目录缓存了结果，用 `west build -p always`。
- 直接修改 `rocket_pi.dts` 做课程实验：不推荐。普通 lesson 优先使用
  应用级 overlay，避免影响其他课程。
- GPIO 有效电平判断反了：不要手动反转低电平 LED，使用
  `gpio_pin_set_dt()` 写逻辑状态。

## 课后练习

1. 把 `lesson-led` 从 `&blue_led` 改成 `&red_led`，重新构建并观察串口输出。
2. 在 `zephyr.dts` 中找到 `lesson-led`，确认它最终指向哪个节点。
3. 删除 `lesson-button` alias，观察编译期错误信息。
4. 思考：为什么 overlay 适合 lesson 和产品差异配置，而不是把所有变化写进
   board DTS？

## 下一课衔接

下一课 `05_kconfig_prjconf` 会继续沿着“配置不写死在 C 代码里”的思路，
学习 `prj.conf` 和 Kconfig 如何控制功能开关、驱动依赖和编译选项。
