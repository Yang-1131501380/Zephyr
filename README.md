# Rocket-Pi Zephyr Learning

这是 Rocket-Pi STM32F401RE 开发板的 Zephyr 学习仓库。仓库采用“一个课程
一个独立应用”的方式组织，每一课都保留完整代码、构建配置和学习说明，
方便逐步回看。

## 固定参考地址

- Zephyr 官方文档：<https://docs.zephyrproject.org/latest/>
- Zephyr 官方 samples：`/home/msy/zephyrproject/zephyr/samples`
- 开发板例程仓库：<https://github.com/newbie-jiang/stm32f401re_rocketpi>
- 个人远程 GitHub 学习仓库：<https://github.com/Yang-1131501380/Zephyr>
- 本地学习仓库：`/home/msy/zephyrproject/apps/rocketpi_zephyr_learning`

## 仓库结构

```text
boards/others/rocket_pi/     # Rocket-Pi 自定义板级支持
docs/                         # 学习工作流、硬件资源和路线记录
lessons/                     # Zephyr 学习课程
  01_blinky/                 # 第一课：GPIO LED 闪烁
  02_button_polling/         # 第二课：按键轮询输入
  03_button_interrupt/       # 第三课：按键 GPIO 中断
  04_devicetree_overlay/     # 第四课：应用级 Devicetree overlay
  05_kconfig_prjconf/        # 第五课：Kconfig 和 prj.conf
  06_printk_logging/         # 第六课：printk 和 logging
  _template/                 # 新课程模板
```

每个课程目录固定包含：

```text
CMakeLists.txt               # Zephyr 应用入口
prj.conf                     # Kconfig 配置
README.md                    # 示例说明和使用方法
study.md                     # 学习重点和注意事项
src/main.c                   # 示例代码
```

## 通用构建方式

在 west workspace 根目录 `/home/msy/zephyrproject` 下执行：

```sh
.venv/bin/west build -p always -b rocket_pi \
  apps/rocketpi_zephyr_learning/lessons/01_blinky \
  -d build/rocketpi_01_blinky
```

## 通用烧录方式

```sh
.venv/bin/west flash -d build/rocketpi_01_blinky -r openocd
```

## 串口观察

Rocket-Pi 当前控制台使用 USART2，波特率为 115200。

```sh
picocom -b 115200 /dev/ttyACM0
```

## 学习进度

| 课程 | 状态 | 重点 | 记录 |
| --- | --- | --- | --- |
| `01_blinky` | 已上板验证 | GPIO、Devicetree alias、串口 `printk()` | RGB LED 按红、绿、蓝循环点亮，当前点亮时间为 100 ms。 |
| `02_button_polling` | 已烧录，待串口验证 | GPIO input、按键有效电平、轮询 | 使用 PA0 用户按键控制 LED，并打印按键状态变化。 |
| `03_button_interrupt` | 已烧录，待串口验证 | GPIO interrupt、callback、workqueue 消抖 | 使用 PA0 双边沿中断触发延时 work，再读取稳定按键状态。 |
| `04_devicetree_overlay` | 已烧录并串口验证 | DTS、overlay、alias、chosen、生成后的 `zephyr.dts` | 使用本课 `app.overlay` 将 lesson LED 指向红灯，按键控制 LED。 |
| `05_kconfig_prjconf` | 已烧录并串口验证 | `prj.conf`、Kconfig、`CONFIG_*`、`IS_ENABLED()` | 使用 lesson 自定义 Kconfig 控制 LED 闪烁周期、按键门控和计数日志。 |
| `06_printk_logging` | 已烧录并串口验证 | `printk()`、logging、日志等级、UART 后端 | 使用绿色 LED 心跳，对比 `printk()` 和 `LOG_INF/WRN/DBG` 串口输出。 |

学习路线采用“双轨”：

- 先看 Zephyr 官方 sample，理解 Zephyr 原生 API 和推荐结构。
- 再看开发板例程仓库，确认 Rocket-Pi 的外设、引脚和已有实现。
- 最后做本仓库 `lessons/`，把同一个知识点落到 Rocket-Pi 实际外设上。

推荐先读：

- `docs/learning-roadmap.md`：完整学习路线、官方 sample、Rocket-Pi 例程和
  lesson 对照。
- `docs/hardware.md`：Rocket-Pi 外设、引脚和课程对应关系。
- `docs/workflow.md`：Linux/Windows 构建、烧录、串口和 Git 记录流程。

## 学习建议

每一课按以下顺序学习：

1. 阅读课程 `README.md`，先明确示例目标和运行现象。
2. 阅读课程 `study.md`，理解本课涉及的 Zephyr 概念。
3. 构建并烧录示例。
4. 修改一个小参数重新验证，例如闪烁周期、输出文本或外设配置。
5. 完成后提交一次 Git 记录。
6. 同步到个人远程 GitHub：<https://github.com/Yang-1131501380/Zephyr>。
