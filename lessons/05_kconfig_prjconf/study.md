# Lesson 05 Study Notes

本课的核心问题是：一个功能为什么由 `CONFIG_` 打开？答案不是“因为
`prj.conf` 写了它”，而是 Kconfig 先定义了这个配置项，`prj.conf` 再给它
赋值，最后构建系统把结果生成到 `.config` 和 `autoconf.h`。

## 学习目标

- 理解 Kconfig、`prj.conf`、`.config`、`autoconf.h` 的关系。
- 能定义应用自己的 Kconfig 选项。
- 能在 C 代码中读取 `CONFIG_*` 常量和布尔开关。
- 理解 `IS_ENABLED()` 相比直接 `#ifdef` 的优势。
- 能通过构建产物追踪一个配置值是否生效。

## 配置流向

本课配置链路：

```text
lessons/05_kconfig_prjconf/Kconfig
  -> 定义 CONFIG_LESSON_05_BLINK_INTERVAL_MS 等选项

lessons/05_kconfig_prjconf/prj.conf
  -> 设置 CONFIG_LESSON_05_BLINK_INTERVAL_MS=300

build/rocketpi_05_kconfig_prjconf/zephyr/.config
  -> 记录最终解析结果

build/rocketpi_05_kconfig_prjconf/zephyr/include/generated/zephyr/autoconf.h
  -> 给 C 代码使用的宏
```

## Kconfig 文件

本课定义三个配置：

```text
CONFIG_LESSON_05_BLINK_INTERVAL_MS
CONFIG_LESSON_05_USE_BUTTON_GATE
CONFIG_LESSON_05_PRINT_COUNTER
```

`BLINK_INTERVAL_MS` 是 `int`，带 `range 50 2000`。如果在 `prj.conf` 里写
超出范围的值，Kconfig 会给出警告或修正，不能把任意数直接塞进 C 代码。

`USE_BUTTON_GATE` 和 `PRINT_COUNTER` 是 `bool`。它们适合控制某段代码是否
编译进固件。

应用级 `Kconfig` 要先包含 Zephyr 的主配置入口：

```text
source "Kconfig.zephyr"
```

如果漏掉这一行，构建会只解析本应用的 Kconfig，`CONFIG_GPIO`、
`CONFIG_SERIAL`、`CONFIG_PRINTK` 等 Zephyr 基础符号都会变成 undefined。

## prj.conf

`prj.conf` 中的配置：

```text
CONFIG_LESSON_05_BLINK_INTERVAL_MS=300
CONFIG_LESSON_05_USE_BUTTON_GATE=y
CONFIG_LESSON_05_PRINT_COUNTER=y
```

注意：`prj.conf` 不负责“声明”配置项。若没有对应 Kconfig 定义，Zephyr 会
把它当成 unknown symbol。

## C 代码写法

整数配置可以直接使用：

```c
k_sleep(K_MSEC(CONFIG_LESSON_05_BLINK_INTERVAL_MS));
```

布尔配置建议用 `IS_ENABLED()`：

```c
if (IS_ENABLED(CONFIG_LESSON_05_USE_BUTTON_GATE)) {
	/* runtime expression */
}
```

当需要完全裁剪代码，仍可以使用预处理：

```c
#if IS_ENABLED(CONFIG_LESSON_05_PRINT_COUNTER)
printk("Blink %u\n", counter);
#endif
```

本课同时展示了两种写法。

## 与 Devicetree 的边界

Devicetree 回答“硬件是什么、接在哪里”：

```c
#define LED_NODE DT_ALIAS(led0)
static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED_NODE, gpios);
```

Kconfig 回答“功能要不要编译、参数默认是多少”：

```c
CONFIG_LESSON_05_USE_BUTTON_GATE
CONFIG_LESSON_05_BLINK_INTERVAL_MS
```

不要把硬件引脚写进 Kconfig，也不要把功能开关塞进 Devicetree。

## 构建检查

构建后执行：

```sh
grep -n "LESSON_05" build/rocketpi_05_kconfig_prjconf/zephyr/.config
```

预期能看到：

```text
CONFIG_LESSON_05_BLINK_INTERVAL_MS=300
CONFIG_LESSON_05_USE_BUTTON_GATE=y
CONFIG_LESSON_05_PRINT_COUNTER=y
```

再检查：

```sh
grep -n "LESSON_05" \
  build/rocketpi_05_kconfig_prjconf/zephyr/include/generated/zephyr/autoconf.h
```

这里是 C 编译器实际看到的宏。

## 常见错误

- 只改 `prj.conf`，没有 Kconfig 定义：会出现 unknown symbol。
- 配置名写错：`CONFIG_LESSON05_*` 和 `CONFIG_LESSON_05_*` 是不同名字。
- 修改配置后复用旧 build：结果可能不符合预期，学习阶段用 `-p always`。
- 用 Kconfig 描述 PA1、PA0 等硬件连接：这是 Devicetree 的职责。

## 课后练习

1. 把 `CONFIG_LESSON_05_BLINK_INTERVAL_MS` 改成 `1000`，观察闪烁周期。
2. 把 `CONFIG_LESSON_05_USE_BUTTON_GATE=n`，确认不按按键也会闪烁。
3. 把 `CONFIG_LESSON_05_PRINT_COUNTER=n`，确认固件不再打印每次 blink。
4. 故意把 `CONFIG_LESSON_05_BLINK_INTERVAL_MS=20`，观察 Kconfig 对 range
   的处理。

## 下一课衔接

下一课 `06_printk_logging` 会进入调试输出能力：从简单 `printk()` 过渡到
Zephyr logging 子系统，学习模块日志、日志等级和运行期过滤。
