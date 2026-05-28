/*
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <stddef.h>

#include <zephyr/drivers/gpio.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/util.h>

#define LED_ON_TIME_MS (500U)

static const struct gpio_dt_spec leds[] = {
	GPIO_DT_SPEC_GET(DT_ALIAS(led0), gpios),
	GPIO_DT_SPEC_GET(DT_ALIAS(led1), gpios),
	GPIO_DT_SPEC_GET(DT_ALIAS(led2), gpios),
};

static const char *const led_names[] = {
	"red",
	"green",
	"blue",
};

BUILD_ASSERT(ARRAY_SIZE(leds) == ARRAY_SIZE(led_names),
	     "LED table and name table size mismatch");

static int configure_leds(void)
{
	for (size_t i = 0U; i < ARRAY_SIZE(leds); i++) {
		int ret;

		if (!gpio_is_ready_dt(&leds[i])) {
			printk("LED %u is not ready\n", (unsigned int)i);
			return -ENODEV;
		}

		ret = gpio_pin_configure_dt(&leds[i], GPIO_OUTPUT_INACTIVE);
		if (ret < 0) {
			printk("Failed to configure LED %u\n",
			       (unsigned int)i);
			return ret;
		}
	}

	return 0;
}

static int show_led(size_t led_index)
{
	int ret;

	printk("LED: %s\n", led_names[led_index]);

	ret = gpio_pin_set_dt(&leds[led_index], 1);
	if (ret < 0) {
		printk("Failed to turn on LED %u\n", (unsigned int)led_index);
		return ret;
	}

	k_msleep(LED_ON_TIME_MS);

	ret = gpio_pin_set_dt(&leds[led_index], 0);
	if (ret < 0) {
		printk("Failed to turn off LED %u\n", (unsigned int)led_index);
		return ret;
	}

	return 0;
}

int main(void)
{
	if (configure_leds() != 0) {
		return 0;
	}

	while (1) {
		for (size_t i = 0U; i < ARRAY_SIZE(leds); i++) {
			if (show_led(i) != 0) {
				return 0;
			}
		}
	}

	return 0;
}
