# Lesson 07 Study Notes

本课从“看串口输出”进入“应用主动读写 UART”。示例直接使用 board 的
`zephyr,console`，因此和 `printk()` 共用同一个 USART2。

## 核心 API

- `DEVICE_DT_GET()`：从 devicetree 节点获取设备对象。
- `device_is_ready()`：确认 UART 驱动已初始化。
- `uart_poll_in()`：非阻塞读取一个字符，没有数据时返回负值。
- `uart_poll_out()`：阻塞发送一个字符。

## 程序流程

1. 获取 `DT_CHOSEN(zephyr_console)` 对应 UART。
2. 打印课程说明。
3. 循环调用 `uart_poll_in()`。
4. 收到普通字符就用 `uart_poll_out()` 回显。
5. 收到回车或换行时输出 CRLF。

## 注意

这个 lesson 复用 console UART，适合学习 polling API。后续做独立应用 UART
时，应优先在 Devicetree 中单独选择 UART 节点，避免和 shell 或日志争用。
