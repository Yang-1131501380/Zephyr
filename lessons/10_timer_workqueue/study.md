# Lesson 10 Study Notes

本课把“定时触发”和“实际处理”拆开：`k_timer` 负责周期性触发，
`k_work` 负责在 workqueue 上执行 GPIO 和打印。

## 重点

- `k_timer_init()`
- `k_timer_start()`
- `k_work_init()`
- `k_work_submit()`
- timer 回调上下文与 workqueue 上下文的边界

## 程序流程

1. 初始化蓝色 LED。
2. 初始化 work，handler 中 toggle LED 并打印。
3. 初始化 timer，timer handler 中递增计数并提交 work。
4. 启动 500 ms 周期 timer。
5. `main()` 进入永久睡眠。

## 注意

不要在 timer 回调中做复杂工作。把耗时处理放进 workqueue，后续按键中断、
传感器采集和通信任务都会用到同样思路。
