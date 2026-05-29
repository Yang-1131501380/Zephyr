/*
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <stdbool.h>
#include <stdint.h>

#include <zephyr/drivers/gpio.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>

#define LED_NODE DT_ALIAS(led0)
#define BUTTON_NODE DT_ALIAS(sw0)

#if !DT_NODE_HAS_STATUS(LED_NODE, okay)
#error "Unsupported board: led0 devicetree alias is not defined"
#endif

#if IS_ENABLED(CONFIG_LESSON_05_USE_BUTTON_GATE) && \
	!DT_NODE_HAS_STATUS(BUTTON_NODE, okay)
#error "Unsupported board: sw0 devicetree alias is not defined"
#endif

static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED_NODE, gpios);

#if IS_ENABLED(CONFIG_LESSON_05_USE_BUTTON_GATE)
static const struct gpio_dt_spec button =
	GPIO_DT_SPEC_GET(BUTTON_NODE, gpios);
#endif

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

#if IS_ENABLED(CONFIG_LESSON_05_USE_BUTTON_GATE)
	if (!gpio_is_ready_dt(&button)) {
		printk("Button GPIO is not ready\n");
		return -ENODEV;
	}

	ret = gpio_pin_configure_dt(&button, GPIO_INPUT);
	if (ret < 0) {
		printk("Failed to configure button: %d\n", ret);
		return ret;
	}
#endif

	return 0;
}

static bool blink_is_enabled(void)
{
#if IS_ENABLED(CONFIG_LESSON_05_USE_BUTTON_GATE)
	int pressed = gpio_pin_get_dt(&button);

	if (pressed < 0) {
		printk("Failed to read button: %d\n", pressed);
		return false;
	}

	return pressed != 0;
#else
	return true;
#endif
}

int main(void)
{
	uint32_t counter = 0U;
	bool led_is_on = false;
	int ret;

	ret = configure_gpio();
	if (ret < 0) {
		return 0;
	}

	printk("Lesson 05: Kconfig and prj.conf\n");
	printk("Blink interval: %d ms\n", CONFIG_LESSON_05_BLINK_INTERVAL_MS);
	printk("Button gate: %s\n",
	       IS_ENABLED(CONFIG_LESSON_05_USE_BUTTON_GATE) ? "enabled" :
							       "disabled");
	printk("Counter log: %s\n",
	       IS_ENABLED(CONFIG_LESSON_05_PRINT_COUNTER) ? "enabled" :
							     "disabled");
	printk("LED: %s pin %u, active %s\n",
	       led.port->name,
	       (unsigned int)led.pin,
	       led.dt_flags & GPIO_ACTIVE_LOW ? "low" : "high");

#if IS_ENABLED(CONFIG_LESSON_05_USE_BUTTON_GATE)
	printk("Hold the user button to allow LED blinking.\n");
#endif

	while (1) {
		if (blink_is_enabled()) {
			led_is_on = !led_is_on;
			(void)gpio_pin_set_dt(&led, led_is_on ? 1 : 0);
			counter++;

#if IS_ENABLED(CONFIG_LESSON_05_PRINT_COUNTER)
			printk("Blink %u: LED %s\n",
			       (unsigned int)counter,
			       led_is_on ? "on" : "off");
#endif
		} else {
			if (led_is_on) {
				led_is_on = false;
				(void)gpio_pin_set_dt(&led, 0);
			}
		}

		k_sleep(K_MSEC(CONFIG_LESSON_05_BLINK_INTERVAL_MS));
	}

	return 0;
}
