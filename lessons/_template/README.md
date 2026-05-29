# Lesson XX: Title

## 示例目标

- 写清本课要验证的硬件现象。
- 写清本课对应的 Zephyr 官方 sample 或 API。
- 写清本课对应的 Rocket-Pi 开发板例程。

## 参考资料

- Zephyr 官方文档：TODO
- Zephyr 官方 sample：TODO
- Rocket-Pi 开发板例程仓库：<https://github.com/newbie-jiang/stm32f401re_rocketpi>
- 对应 Rocket-Pi 例程：TODO
- 个人远程 GitHub 学习仓库：<https://github.com/Yang-1131501380/Zephyr>

## 硬件资源

| 功能 | Zephyr 节点或 alias | 引脚/外设 | 说明 |
| --- | --- | --- | --- |
| TODO | TODO | TODO | TODO |

## 构建

在 west workspace 根目录 `/home/msy/zephyrproject` 下执行：

```sh
CCACHE_DISABLE=1 .venv/bin/west build -p always -b rocket_pi \
  apps/rocketpi_zephyr_learning/lessons/<lesson_name> \
  -d build/rocketpi_<lesson_name>
```

## 烧录

```sh
.venv/bin/west flash -d build/rocketpi_<lesson_name> -r openocd
```

## 串口

```sh
picocom -b 115200 /dev/ttyACM0
```

## 预期现象

- 串口输出：

```text
TODO
```

## 上板验证记录

- 构建状态：TODO
- 烧录状态：TODO
- 串口输出：TODO
- 硬件现象：TODO
