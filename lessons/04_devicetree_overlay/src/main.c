/*
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <stdbool.h>

#include <zephyr/devicetree.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>

#define LED_NODE DT_ALIAS(lesson_led)
#define BUTTON_NODE DT_ALIAS(lesson_button)
#define CHOSEN_LED_NODE DT_CHOSEN(zephyr_lesson_led)
#define CHOSEN_BUTTON_NODE DT_CHOSEN(zephyr_lesson_button)

#if !DT_NODE_HAS_STATUS(LED_NODE, okay)
#error "lesson-led alias is missing or disabled; check app.overlay"
#endif

#if !DT_NODE_HAS_STATUS(BUTTON_NODE, okay)
#error "lesson-button alias is missing or disabled; check app.overlay"
#endif

#if !DT_NODE_HAS_STATUS(CHOSEN_LED_NODE, okay)
#error "zephyr,lesson-led chosen node is missing or disabled; check app.overlay"
#endif

#if !DT_NODE_HAS_STATUS(CHOSEN_BUTTON_NODE, okay)
#error "zephyr,lesson-button chosen node is missing or disabled; check app.overlay"
#endif

static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED_NODE, gpios);
static const struct gpio_dt_spec button =
	GPIO_DT_SPEC_GET(BUTTON_NODE, gpios);

static int configure_gpio(void)
{
	int ret;

	if (!gpio_is_ready_dt(&led)) {
		printk("Overlay LED GPIO is not ready\n");
		return -ENODEV;
	}

	if (!gpio_is_ready_dt(&button)) {
		printk("Overlay button GPIO is not ready\n");
		return -ENODEV;
	}

	ret = gpio_pin_configure_dt(&led, GPIO_OUTPUT_INACTIVE);
	if (ret < 0) {
		printk("Failed to configure overlay LED: %d\n", ret);
		return ret;
	}

	ret = gpio_pin_configure_dt(&button, GPIO_INPUT);
	if (ret < 0) {
		printk("Failed to configure overlay button: %d\n", ret);
		return ret;
	}

	return 0;
}

static const char *gpio_active_level(const struct gpio_dt_spec *spec)
{
	return spec->dt_flags & GPIO_ACTIVE_LOW ? "low" : "high";
}

int main(void)
{
	int last_pressed = -1;
	int ret;

	ret = configure_gpio();
	if (ret < 0) {
		return 0;
	}

	printk("Lesson 04: devicetree overlay\n");
	printk("lesson-led: %s pin %u, active %s\n",
	       led.port->name,
	       (unsigned int)led.pin,
	       gpio_active_level(&led));
	printk("lesson-button: %s pin %u, active %s\n",
	       button.port->name,
	       (unsigned int)button.pin,
	       gpio_active_level(&button));
	printk("Overlay selected LED label: %s\n", DT_PROP(LED_NODE, label));
	printk("Overlay selected button label: %s\n",
	       DT_PROP(BUTTON_NODE, label));

	while (1) {
		bool is_pressed;
		int pressed = gpio_pin_get_dt(&button);

		if (pressed < 0) {
			printk("Failed to read overlay button: %d\n", pressed);
			k_sleep(K_MSEC(500));
			continue;
		}

		is_pressed = pressed != 0;
		(void)gpio_pin_set_dt(&led, is_pressed ? 1 : 0);

		if (pressed != last_pressed) {
			last_pressed = pressed;
			printk("Button %s -> overlay LED %s\n",
			       is_pressed ? "pressed" : "released",
			       is_pressed ? "on" : "off");
		}

		k_sleep(K_MSEC(20));
	}

	return 0;
}
