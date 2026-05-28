# Rocket-Pi Zephyr Learning

这是 Rocket-Pi STM32F401RE 开发板的 Zephyr 学习仓库。仓库采用
“一个课程一个独立应用”的方式组织，每一课都保留完整代码、构建配置和
学习说明，方便逐步回看。

## 仓库结构

```text
boards/others/rocket_pi/     # Rocket-Pi 自定义板级支持
lessons/                     # Zephyr 学习课程
  01_blinky/                 # 第一课：GPIO LED 闪烁
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

下一课建议学习按键输入：先用轮询读取 GPIO input，再升级到 GPIO interrupt。
这样可以把本课的 Devicetree/GPIO 输出知识自然扩展到输入和中断。

## 学习建议

每一课按以下顺序学习：

1. 阅读课程 `README.md`，先明确示例目标和运行现象。
2. 阅读课程 `study.md`，理解本课涉及的 Zephyr 概念。
3. 构建并烧录示例。
4. 修改一个小参数重新验证，例如闪烁周期、输出文本或外设配置。
5. 完成后提交一次 Git 记录。
