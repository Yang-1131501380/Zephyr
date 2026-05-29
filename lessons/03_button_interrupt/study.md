# Lesson 03 Study Notes

本课从第二课的轮询方式升级到 GPIO 中断。重点不是“中断更高级”，而是
理解中断上下文和普通线程上下文的边界。

## 学习目标

- 掌握 Zephyr GPIO 中断配置流程。
- 理解 `gpio_callback` 的作用和生命周期。
- 理解为什么中断回调中不能做复杂逻辑。
- 掌握一种简单的软件消抖方式：中断触发后延时读取稳定电平。
- 对比轮询、中断、workqueue 三者的职责。

## 官方 sample

先阅读 Zephyr 官方 `samples/basic/button`。

官方 sample 可能使用 input subsystem。本课仍然使用 GPIO API，是为了把
第二课的 `gpio_dt_spec` 学习路线延续下来。等 GPIO 中断理解清楚后，再
学习 input subsystem 会更自然。

## Rocket-Pi 对照例程

- 开发板例程仓库：<https://github.com/newbie-jiang/stm32f401re_rocketpi>
- 对应例程：`rocketpi_key_irq`

对照时重点看：

1. 裸机/RTOS 例程如何配置按键中断。
2. 中断服务函数里做了哪些事。
3. Zephyr 版本如何用 `gpio_callback` 代替直接写 EXTI ISR。

## Devicetree

本课继续使用 Rocket-Pi board 中已有的 alias：

```dts
aliases {
	sw0 = &user_button;
	led0 = &red_led;
};
```

按键节点：

```dts
user_button: button {
	label = "User";
	gpios = <&gpioa 0 (GPIO_PULL_DOWN | GPIO_ACTIVE_HIGH)>;
	zephyr,code = <INPUT_KEY_0>;
};
```

代码依然不直接写 PA0：

```c
#define BUTTON_NODE DT_ALIAS(sw0)

static const struct gpio_dt_spec button =
	GPIO_DT_SPEC_GET(BUTTON_NODE, gpios);
```

## prj.conf

```text
CONFIG_GPIO=y
CONFIG_SERIAL=y
CONFIG_CONSOLE=y
CONFIG_UART_CONSOLE=y
CONFIG_PRINTK=y
CONFIG_SYSTEM_WORKQUEUE_STACK_SIZE=1024
```

说明：

- `CONFIG_GPIO=y`：启用 GPIO 输入、输出和中断相关 API。
- 串口和 console 配置用于 `printk()` 输出。
- `CONFIG_SYSTEM_WORKQUEUE_STACK_SIZE=1024`：本课使用系统 workqueue 执行
  消抖后的按键处理。当前逻辑很轻量，1024 字节足够。

## 核心 API

### `gpio_pin_interrupt_configure_dt()`

启用按键中断：

```c
gpio_pin_interrupt_configure_dt(&button, GPIO_INT_EDGE_BOTH);
```

`GPIO_INT_EDGE_BOTH` 表示按下和松开都触发。对本课来说，这样可以同时
检测 pressed 和 released。

### `gpio_init_callback()`

初始化回调对象：

```c
gpio_init_callback(&button_callback,
		   button_isr_callback,
		   BIT(button.pin));
```

第三个参数是 pin mask，表示这个 callback 关心哪个引脚。

### `gpio_add_callback()`

把 callback 注册到 GPIO 控制器：

```c
gpio_add_callback(button.port, &button_callback);
```

`button_callback` 必须是静态或全局对象，不能是局部变量。否则函数返回后
对象生命周期结束，中断来时会访问无效内存。

### `k_work_delayable`

本课使用延时 work 做消抖：

```c
k_work_init_delayable(&button_work, button_work_handler);
k_work_reschedule(&button_work, K_MSEC(BUTTON_DEBOUNCE_DELAY_MS));
```

按键抖动时会连续触发多个边沿，每次中断都重新安排 30 ms 后执行 work。
最后只读取一次稳定电平。

## Zephyr GPIO 中断如何映射到 STM32 硬件

本课代码看起来只调用 Zephyr GPIO API：

```c
gpio_pin_interrupt_configure_dt(&button, GPIO_INT_EDGE_BOTH);
```

但在 STM32F401RE 上，它最终会落到 STM32 的 EXTI 外部中断控制器和 NVIC。
以 Rocket-Pi 的 PA0 用户按键为例，完整路径是：

```text
PA0 按键电平变化
  -> GPIOA pin 0
  -> EXTI line 0
  -> NVIC EXTI0_IRQn
  -> Zephyr STM32 EXTI/GPIO interrupt driver
  -> gpio_callback
  -> button_isr_callback()
  -> k_work_delayable 消抖处理
```

### Devicetree 到 GPIO 设备

`rocket_pi.dts` 中的按键定义是：

```dts
user_button: button {
	label = "User";
	gpios = <&gpioa 0 (GPIO_PULL_DOWN | GPIO_ACTIVE_HIGH)>;
	zephyr,code = <INPUT_KEY_0>;
};

aliases {
	sw0 = &user_button;
};
```

所以代码中的：

```c
GPIO_DT_SPEC_GET(DT_ALIAS(sw0), gpios)
```

会在编译期生成一个 `gpio_dt_spec`，里面包含：

```text
button.port     = GPIOA 设备
button.pin      = 0
button.dt_flags = GPIO_PULL_DOWN | GPIO_ACTIVE_HIGH
```

应用代码不直接写 PA0，而是通过 `sw0` 找到 PA0。这样后续如果换板或换
按键引脚，优先改 Devicetree，不改应用逻辑。

### GPIO 输入配置

本课先调用：

```c
gpio_pin_configure_dt(&button, GPIO_INPUT);
```

它最终进入 Zephyr 的 STM32 GPIO 驱动：

```text
zephyr/drivers/gpio/gpio_stm32.c
```

驱动会把 Zephyr 的 `GPIO_INPUT | GPIO_PULL_DOWN` 转换成 STM32 GPIO
寄存器配置。对 PA0 来说，大致对应：

```text
GPIOA_MODER -> input mode
GPIOA_PUPDR -> pull-down
```

也就是把 PA0 配成带下拉的输入脚。

### EXTI 线和端口来源

STM32 的 GPIO 外部中断不是“每个引脚一个独立中断”。它通过 EXTI line
工作。PA0、PB0、PC0 都对应 EXTI line 0，但同一时间 EXTI0 只能选择一个
端口来源。

本课用的是 PA0，所以映射关系是：

```text
PA0 -> EXTI0
```

`gpio_pin_interrupt_configure_dt()` 在 STM32 GPIO 驱动里会做三类事情：

```text
1. 选择 EXTI0 的来源端口是 GPIOA
2. 配置 EXTI0 的触发边沿
3. 使能 EXTI0 对应的 NVIC 中断
```

对应到 STM32F401RE 的硬件寄存器概念，大致是：

```text
SYSCFG_EXTICR[0] 选择 EXTI0 来源为 PA0
EXTI_RTSR        使能 EXTI0 上升沿触发
EXTI_FTSR        使能 EXTI0 下降沿触发
EXTI_IMR         允许 EXTI0 产生 interrupt
NVIC             使能 EXTI0_IRQn
```

`GPIO_INT_EDGE_BOTH` 就是让上升沿和下降沿都触发。对本课按键来说：

```text
松开 -> 按下：PA0 从 0 变 1，上升沿
按下 -> 松开：PA0 从 1 变 0，下降沿
```

所以按下和松开都会进入中断链路。

### EXTI0 对应哪个 NVIC IRQ

STM32F4 的 Zephyr DTS 已经描述了 EXTI 中断分组：

```dts
exti: interrupt-controller@40013c00 {
	compatible = "st,stm32-exti";
	interrupts = <6 0>, <7 0>, <8 0>, <9 0>,
		     <10 0>, <23 0>, <40 0>;
	interrupt-names = "line0", "line1", "line2", "line3",
			  "line4", "line5-9", "line10-15";
	line-ranges = <0 1>, <1 1>, <2 1>, <3 1>,
		      <4 1>, <5 5>, <10 6>;
};
```

对 PA0 来说：

```text
PA0 -> EXTI line 0 -> line0 -> IRQ number 6 -> EXTI0_IRQn
```

常见分组可以这样记：

```text
PA0       -> EXTI0_IRQn
PA1       -> EXTI1_IRQn
PA2       -> EXTI2_IRQn
PA3       -> EXTI3_IRQn
PA4       -> EXTI4_IRQn
PA5~PA9   -> EXTI9_5_IRQn
PA10~PA15 -> EXTI15_10_IRQn
```

### Zephyr 如何调用你的 callback

你注册 callback 的代码是：

```c
gpio_init_callback(&button_callback,
		   button_isr_callback,
		   BIT(button.pin));

gpio_add_callback(button.port, &button_callback);
```

`BIT(button.pin)` 对 PA0 来说就是：

```text
BIT(0) = 0x00000001
```

表示这个 callback 只关心 GPIOA 的 pin 0。

按下 PA0 后，硬件和软件调用链大致是：

```text
PA0 电平变化
  -> EXTI0 pending bit 被置位
  -> NVIC 进入 EXTI0_IRQn
  -> Zephyr 的 STM32 EXTI ISR 检查 EXTI0 pending
  -> 清除 EXTI0 pending
  -> STM32 GPIO driver 调用 gpio_fire_callbacks()
  -> Zephyr GPIO callback 列表匹配 BIT(0)
  -> 调用 button_isr_callback()
```

所以你写的 `button_isr_callback()` 不是直接绑定到 STM32 的
`EXTI0_IRQHandler`，而是通过 Zephyr 的 GPIO/EXTI 驱动分发后被调用。
这样应用层代码可以保持跨板、跨 SoC 的 Zephyr 风格。

## 程序流程

1. 编译期确认 `sw0` 和 `led0` alias 存在。
2. 配置按键为输入，配置 LED 为输出 inactive。
3. 初始化 `button_work`。
4. 初始化并注册 `button_callback`。
5. 启用 PA0 双边沿中断。
6. 启动时读取一次当前按键状态，同步 LED。
7. 按键电平变化时进入 GPIO callback。
8. callback 只重启延时 work，不打印、不读按键、不控制 LED。
9. 30 ms 后 workqueue 执行 `process_button_state()`。
10. 读取稳定电平，状态变化时打印，并同步 LED。

## 为什么不在中断回调里直接处理

中断回调应尽量短，原因：

- 中断上下文不适合做耗时逻辑。
- 串口打印可能比较慢，不适合放在中断里。
- 复杂业务放在中断里会影响系统实时性。
- 以后如果按键事件要发消息队列或更新 UI，放到线程上下文更容易扩展。

本课的做法是：

```text
GPIO edge interrupt -> callback -> delayable work -> read stable state
```

这比“callback 里直接控制所有业务”更稳妥。

## 轮询和中断对比

| 方式 | 优点 | 缺点 | 适合场景 |
| --- | --- | --- | --- |
| 轮询 | 简单、容易理解 | 占用周期检查，响应受周期影响 | 入门、低频状态检查 |
| 中断 | 响应及时，不用持续检查 | 需要处理回调上下文和抖动 | 按键、外部事件 |
| 中断 + work | ISR 极简，业务在任务上下文 | 结构比轮询多一步 | 推荐的工程化入门方式 |

## 常见问题

### 按一下打印多次

机械按键会抖动。先把 `BUTTON_DEBOUNCE_DELAY_MS` 从 30 改到 50，再观察。

### 按键只按下有反应，松开没反应

检查中断配置是否为：

```c
GPIO_INT_EDGE_BOTH
```

如果改成 rising 或 falling，就只会响应一个方向。

### 构建找不到 `sw0`

检查 `rocket_pi.dts` 是否包含：

```dts
aliases {
	sw0 = &user_button;
};
```

### work 没有执行

检查是否调用了：

```c
k_work_init_delayable(&button_work, button_work_handler);
```

并确认 callback 中调用了 `k_work_reschedule()`。

## 课后练习

1. 把消抖时间从 30 ms 改成 10 ms，观察是否更容易重复触发。
2. 改成按下一次切换 LED 状态，而不是按下亮、松开灭。
3. 统计按下次数，只在 released 到 pressed 的边沿加一。
4. 把 `GPIO_INT_EDGE_BOTH` 改成 `GPIO_INT_EDGE_RISING`，观察松开是否还打印。
5. 尝试把串口打印放进 callback，再思考为什么不推荐这样做。

## 下一课衔接

下一课 `04_devicetree_overlay` 建议学习：

- board 主 DTS 和 lesson overlay 的边界。
- 如何在课程中局部调整 alias 或启用节点。
- 如何查看最终生成的 `zephyr.dts`。
