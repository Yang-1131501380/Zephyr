/*
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <stdint.h>

#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>

#define UART_NODE DT_CHOSEN(zephyr_console)

#if !DT_NODE_HAS_STATUS(UART_NODE, okay)
#error "Unsupported board: zephyr,console UART is not defined"
#endif

static const struct device *const uart = DEVICE_DT_GET(UART_NODE);

static void uart_write_string(const char *text)
{
	for (const char *p = text; *p != '\0'; p++) {
		uart_poll_out(uart, *p);
	}
}

int main(void)
{
	unsigned char ch;

	if (!device_is_ready(uart)) {
		printk("UART device is not ready\n");
		return 0;
	}

	printk("Lesson 07: UART echo\n");
	printk("Console UART: %s\n", uart->name);
	printk("Type characters in the serial terminal. Press Enter for newline.\n");

	while (1) {
		if (uart_poll_in(uart, &ch) == 0) {
			if (ch == '\r') {
				uart_write_string("\r\n");
			} else if (ch == '\n') {
				uart_write_string("\r\n");
			} else {
				uart_poll_out(uart, ch);
			}
		} else {
			k_sleep(K_MSEC(10));
		}
	}

	return 0;
}
