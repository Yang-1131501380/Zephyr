# Lesson 08 Study Notes

本课学习 Zephyr Shell 的最小使用方式：打开 shell 后端，注册命令，在命令
处理函数中解析参数并调用 GPIO API。

## 程序流程

1. 初始化红、绿、蓝三个 LED。
2. 注册 `led` shell 命令。
3. shell 后端接管串口输入。
4. 用户输入 `led <target> <on|off>`。
5. 命令处理函数控制对应 LED。

## 常见错误

- 忘记 `CONFIG_SHELL_BACKEND_SERIAL=y`：shell 不会出现在串口。
- 命令参数数量不对：`SHELL_CMD_ARG_REGISTER()` 的 mandatory 参数要匹配。
- 手动反转低电平 LED：应使用 Zephyr 逻辑状态 API。
