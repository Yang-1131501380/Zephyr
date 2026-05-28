/*
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/drivers/gpio.h>
#include <zephyr/kernel.h>

#define SLEEP_TIME_MS 500

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

int main(void)
{
	for (size_t i = 0; i < ARRAY_SIZE(leds); i++) {
		if (!gpio_is_ready_dt(&leds[i])) {
			printk("LED %u is not ready\n", i);
			return 0;
		}

		if (gpio_pin_configure_dt(&leds[i], GPIO_OUTPUT_INACTIVE) < 0) {
			printk("Failed to configure LED %u\n", i);
			return 0;
		}
	}

	while (1) {
		for (size_t i = 0; i < ARRAY_SIZE(leds); i++) {
			printk("LED: %s\n", led_names[i]);
			gpio_pin_set_dt(&leds[i], 1);
			k_msleep(SLEEP_TIME_MS);
			gpio_pin_set_dt(&leds[i], 0);
		}
	}

	return 0;
}
