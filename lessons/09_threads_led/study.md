# Lesson 09 Study Notes

本课从单一 `while (1)` 进入多线程模型。三个线程共用同一个入口函数，但
通过不同配置结构控制不同 LED 和周期。

## 重点

- `K_THREAD_DEFINE()`
- thread stack size
- thread priority
- `k_sem` 同步启动
- 多线程中避免在 GPIO 未初始化前运行

## 程序流程

1. `K_THREAD_DEFINE()` 创建三个 LED 线程。
2. 线程先阻塞在 `k_sem_take()`。
3. `main()` 配置 GPIO。
4. `main()` 释放三次信号量，让三个线程开始运行。
5. 每个线程按自己的周期 toggle LED。
