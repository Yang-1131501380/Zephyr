# Lesson 10: Timer and Workqueue

本课用 `k_timer` 周期触发事件，但不在 timer 回调里做 GPIO 和串口输出；
timer 回调只提交 `k_work`，真正处理在系统 workqueue 上完成。

## 构建

```sh
CCACHE_DISABLE=1 .venv/bin/west build -p always -b rocket_pi \
  apps/rocketpi_zephyr_learning/lessons/10_timer_workqueue \
  -d build/rocketpi_10_timer_workqueue
```

## 预期现象

- 蓝色 LED 每 500 ms 翻转。
- 串口打印 `Timer tick N handled by workqueue`。

## 本课记录

- 构建产物：`build/rocketpi_10_timer_workqueue/zephyr/zephyr.hex`
- timer 回调中只做计数和 `k_work_submit()`。
- 2026-05-29 已构建通过，待烧录和 timer/workqueue 现象验证。
