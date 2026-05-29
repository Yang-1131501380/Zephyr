/*
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <stdbool.h>

#include <zephyr/drivers/gpio.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>

#define BUTTON_NODE DT_ALIAS(sw0)
#define LED_NODE DT_ALIAS(led0)
#define BUTTON_POLL_INTERVAL_MS (20U)

#if !DT_NODE_HAS_STATUS(BUTTON_NODE, okay)
#error "Unsupported board: sw0 devicetree alias is not defined"
#endif

#if !DT_NODE_HAS_STATUS(LED_NODE, okay)
#error "Unsupported board: led0 devicetree alias is not defined"
#endif

static const struct gpio_dt_spec button =
	GPIO_DT_SPEC_GET(BUTTON_NODE, gpios);
static const struct gpio_dt_spec led =
	GPIO_DT_SPEC_GET(LED_NODE, gpios);

static const char *button_state_to_text(bool is_pressed)
{
	return is_pressed ? "pressed" : "released";
}

static int configure_gpio(void)
{
	int ret;

	if (!gpio_is_ready_dt(&button)) {
		printk("Button GPIO is not ready\n");
		return -ENODEV;
	}

	if (!gpio_is_ready_dt(&led)) {
		printk("LED GPIO is not ready\n");
		return -ENODEV;
	}

	ret = gpio_pin_configure_dt(&button, GPIO_INPUT);
	if (ret < 0) {
		printk("Failed to configure button: %d\n", ret);
		return ret;
	}

	ret = gpio_pin_configure_dt(&led, GPIO_OUTPUT_INACTIVE);
	if (ret < 0) {
		printk("Failed to configure LED: %d\n", ret);
		return ret;
	}

	return 0;
}

static int update_led(bool is_pressed)
{
	int ret;

	ret = gpio_pin_set_dt(&led, is_pressed ? 1 : 0);
	if (ret < 0) {
		printk("Failed to update LED: %d\n", ret);
		return ret;
	}

	return 0;
}

int main(void)
{
	int previous_state = -1;
	int ret;

	ret = configure_gpio();
	if (ret < 0) {
		return 0;
	}

	printk("Lesson 02: button polling\n");
	printk("Button: %s pin %u, active %s\n",
	       button.port->name,
	       (unsigned int)button.pin,
	       button.dt_flags & GPIO_ACTIVE_LOW ? "low" : "high");
	printk("LED: %s pin %u, active %s\n",
	       led.port->name,
	       (unsigned int)led.pin,
	       led.dt_flags & GPIO_ACTIVE_LOW ? "low" : "high");
	printk("Polling interval: %u ms\n", BUTTON_POLL_INTERVAL_MS);

	while (1) {
		int pressed;
		bool is_pressed;

		pressed = gpio_pin_get_dt(&button);
		if (pressed < 0) {
			printk("Failed to read button: %d\n", pressed);
			return 0;
		}

		is_pressed = pressed != 0;

		if (pressed != previous_state) {
			printk("Button: %s\n", button_state_to_text(is_pressed));
			previous_state = pressed;
		}

		ret = update_led(is_pressed);
		if (ret < 0) {
			return 0;
		}

		k_msleep(BUTTON_POLL_INTERVAL_MS);
	}

	return 0;
}
