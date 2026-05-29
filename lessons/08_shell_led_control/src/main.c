/*
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <stdbool.h>
#include <string.h>

#include <zephyr/drivers/gpio.h>
#include <zephyr/kernel.h>
#include <zephyr/shell/shell.h>
#include <zephyr/sys/printk.h>

#define RED_NODE DT_ALIAS(led0)
#define GREEN_NODE DT_ALIAS(led1)
#define BLUE_NODE DT_ALIAS(led2)

#if !DT_NODE_HAS_STATUS(RED_NODE, okay) || \
	!DT_NODE_HAS_STATUS(GREEN_NODE, okay) || \
	!DT_NODE_HAS_STATUS(BLUE_NODE, okay)
#error "Unsupported board: led0, led1 and led2 aliases are required"
#endif

struct named_led {
	const char *name;
	const struct gpio_dt_spec spec;
};

static const struct named_led leds[] = {
	{ "red", GPIO_DT_SPEC_GET(RED_NODE, gpios) },
	{ "green", GPIO_DT_SPEC_GET(GREEN_NODE, gpios) },
	{ "blue", GPIO_DT_SPEC_GET(BLUE_NODE, gpios) },
};

static const struct named_led *find_led(const char *name)
{
	for (size_t i = 0; i < ARRAY_SIZE(leds); i++) {
		if (strcmp(name, leds[i].name) == 0) {
			return &leds[i];
		}
	}

	return NULL;
}

static int set_led(const struct named_led *led, bool is_on)
{
	return gpio_pin_set_dt(&led->spec, is_on ? 1 : 0);
}

static int cmd_led(const struct shell *shell, size_t argc, char **argv)
{
	const struct named_led *led;
	bool is_on;
	int ret;

	if (argc != 3) {
		shell_error(shell, "usage: led <red|green|blue|all> <on|off>");
		return -EINVAL;
	}

	if (strcmp(argv[2], "on") == 0) {
		is_on = true;
	} else if (strcmp(argv[2], "off") == 0) {
		is_on = false;
	} else {
		shell_error(shell, "state must be on or off");
		return -EINVAL;
	}

	if (strcmp(argv[1], "all") == 0) {
		for (size_t i = 0; i < ARRAY_SIZE(leds); i++) {
			ret = set_led(&leds[i], is_on);
			if (ret < 0) {
				shell_error(shell, "failed to set %s: %d",
					    leds[i].name, ret);
				return ret;
			}
		}
		shell_print(shell, "all LEDs %s", is_on ? "on" : "off");
		return 0;
	}

	led = find_led(argv[1]);
	if (led == NULL) {
		shell_error(shell, "unknown LED: %s", argv[1]);
		return -EINVAL;
	}

	ret = set_led(led, is_on);
	if (ret < 0) {
		shell_error(shell, "failed to set %s: %d", led->name, ret);
		return ret;
	}

	shell_print(shell, "%s LED %s", led->name, is_on ? "on" : "off");
	return 0;
}

SHELL_CMD_ARG_REGISTER(led, NULL,
		       "Control RGB LEDs: led <red|green|blue|all> <on|off>",
		       cmd_led, 3, 0);

static int configure_leds(void)
{
	for (size_t i = 0; i < ARRAY_SIZE(leds); i++) {
		int ret;

		if (!gpio_is_ready_dt(&leds[i].spec)) {
			printk("%s LED GPIO is not ready\n", leds[i].name);
			return -ENODEV;
		}

		ret = gpio_pin_configure_dt(&leds[i].spec,
					    GPIO_OUTPUT_INACTIVE);
		if (ret < 0) {
			printk("Failed to configure %s LED: %d\n",
			       leds[i].name, ret);
			return ret;
		}
	}

	return 0;
}

int main(void)
{
	int ret = configure_leds();

	if (ret < 0) {
		return 0;
	}

	printk("Lesson 08: shell LED control\n");
	printk("Try: led red on, led green off, led all off\n");

	while (1) {
		k_sleep(K_FOREVER);
	}

	return 0;
}
