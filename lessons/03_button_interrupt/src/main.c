/*
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <stdbool.h>
#include <stdint.h>

#include <zephyr/drivers/gpio.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>

#define BUTTON_NODE DT_ALIAS(sw0)
#define LED_NODE DT_ALIAS(led0)
#define BUTTON_DEBOUNCE_DELAY_MS (30U)

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

static struct gpio_callback button_callback;
static struct k_work_delayable button_work;
static int stable_button_state = -1;

static const char *button_state_to_text(bool is_pressed)
{
	return is_pressed ? "pressed" : "released";
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

static void process_button_state(void)
{
	int pressed;
	bool is_pressed;

	pressed = gpio_pin_get_dt(&button);
	if (pressed < 0) {
		printk("Failed to read button: %d\n", pressed);
		return;
	}

	is_pressed = pressed != 0;

	if (pressed != stable_button_state) {
		stable_button_state = pressed;
		printk("Button: %s\n", button_state_to_text(is_pressed));
	}

	(void)update_led(is_pressed);
}

static void button_work_handler(struct k_work *work)
{
	(void)work;

	process_button_state();
}

static void button_isr_callback(const struct device *port,
				struct gpio_callback *callback,
				uint32_t pins)
{
	(void)port;
	(void)callback;
	(void)pins;

	(void)k_work_reschedule(&button_work,
				K_MSEC(BUTTON_DEBOUNCE_DELAY_MS));
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

	k_work_init_delayable(&button_work, button_work_handler);

	gpio_init_callback(&button_callback,
			   button_isr_callback,
			   BIT(button.pin));

	ret = gpio_add_callback(button.port, &button_callback);
	if (ret < 0) {
		printk("Failed to add button callback: %d\n", ret);
		return ret;
	}

	ret = gpio_pin_interrupt_configure_dt(&button,
					      GPIO_INT_EDGE_BOTH);
	if (ret < 0) {
		printk("Failed to enable button interrupt: %d\n", ret);
		return ret;
	}

	return 0;
}

int main(void)
{
	int ret;

	ret = configure_gpio();
	if (ret < 0) {
		return 0;
	}

	printk("Lesson 03: button interrupt\n");
	printk("Button: %s pin %u, active %s\n",
	       button.port->name,
	       (unsigned int)button.pin,
	       button.dt_flags & GPIO_ACTIVE_LOW ? "low" : "high");
	printk("LED: %s pin %u, active %s\n",
	       led.port->name,
	       (unsigned int)led.pin,
	       led.dt_flags & GPIO_ACTIVE_LOW ? "low" : "high");
	printk("Debounce delay: %u ms\n", BUTTON_DEBOUNCE_DELAY_MS);

	process_button_state();

	while (1) {
		k_sleep(K_FOREVER);
	}

	return 0;
}
