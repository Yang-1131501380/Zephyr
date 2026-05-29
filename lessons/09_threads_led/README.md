# Lesson 09: Threads LED

本课使用三个 Zephyr thread 分别控制 RGB LED，以不同周期翻转，观察线程、
优先级、栈和同步启动。

## 构建

```sh
CCACHE_DISABLE=1 .venv/bin/west build -p always -b rocket_pi \
  apps/rocketpi_zephyr_learning/lessons/09_threads_led \
  -d build/rocketpi_09_threads_led
```

## 预期现象

- 红灯 250 ms 翻转。
- 绿灯 500 ms 翻转。
- 蓝灯 1000 ms 翻转。
- 串口打印三个线程的 toggle 计数。

## 本课记录

- 构建产物：`build/rocketpi_09_threads_led/zephyr/zephyr.hex`
- 使用 `K_THREAD_DEFINE()` 静态创建线程。
- 使用 `K_SEM_DEFINE()` 静态初始化启动信号量，避免线程早于 `main()` 访问未初始化对象。
- 2026-05-29 已构建通过，待烧录和多线程 LED 现象验证。
