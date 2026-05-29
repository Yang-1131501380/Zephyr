/*
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <stdint.h>

#include <zephyr/drivers/gpio.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/printk.h>

LOG_MODULE_REGISTER(lesson_06, CONFIG_LESSON_06_LOG_LEVEL);

#define LED_NODE DT_ALIAS(led1)

#if !DT_NODE_HAS_STATUS(LED_NODE, okay)
#error "Unsupported board: led1 devicetree alias is not defined"
#endif

static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED_NODE, gpios);

static int configure_gpio(void)
{
	int ret;

	if (!gpio_is_ready_dt(&led)) {
		printk("LED GPIO is not ready\n");
		return -ENODEV;
	}

	ret = gpio_pin_configure_dt(&led, GPIO_OUTPUT_INACTIVE);
	if (ret < 0) {
		printk("Failed to configure LED: %d\n", ret);
		return ret;
	}

	return 0;
}

int main(void)
{
	uint32_t heartbeat = 0U;
	bool led_is_on = false;
	int ret;

	ret = configure_gpio();
	if (ret < 0) {
		return 0;
	}

	printk("Lesson 06: printk and logging\n");
	printk("printk() writes directly to the console path.\n");

	LOG_INF("Logging subsystem is running");
	LOG_INF("Module log level: %d", CONFIG_LESSON_06_LOG_LEVEL);
	LOG_INF("Heartbeat interval: %d ms",
		CONFIG_LESSON_06_HEARTBEAT_INTERVAL_MS);
	LOG_INF("LED: %s pin %u, active %s",
		led.port->name,
		(unsigned int)led.pin,
		led.dt_flags & GPIO_ACTIVE_LOW ? "low" : "high");
	LOG_WRN("This warning is intentional for lesson output");
	LOG_DBG("Debug log is visible because LESSON_06_LOG_LEVEL=4");

	while (1) {
		led_is_on = !led_is_on;
		(void)gpio_pin_set_dt(&led, led_is_on ? 1 : 0);
		heartbeat++;

		LOG_INF("Heartbeat %u: green LED %s",
			(unsigned int)heartbeat,
			led_is_on ? "on" : "off");

		if ((heartbeat % 5U) == 0U) {
			LOG_WRN("Heartbeat reached %u",
				(unsigned int)heartbeat);
		}

		LOG_DBG("Loop iteration complete");

		k_sleep(K_MSEC(CONFIG_LESSON_06_HEARTBEAT_INTERVAL_MS));
	}

	return 0;
}
