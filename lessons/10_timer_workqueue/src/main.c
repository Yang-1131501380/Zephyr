/*
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <stdint.h>

#include <zephyr/drivers/gpio.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>

#define LED_NODE DT_ALIAS(led2)
#define TIMER_PERIOD_MS 500U

#if !DT_NODE_HAS_STATUS(LED_NODE, okay)
#error "Unsupported board: led2 devicetree alias is not defined"
#endif

static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED_NODE, gpios);
static struct k_work led_work;
static struct k_timer led_timer;
static uint32_t timer_ticks;

static void led_work_handler(struct k_work *work)
{
	ARG_UNUSED(work);

	(void)gpio_pin_toggle_dt(&led);
	printk("Timer tick %u handled by workqueue\n",
	       (unsigned int)timer_ticks);
}

static void led_timer_handler(struct k_timer *timer)
{
	ARG_UNUSED(timer);

	timer_ticks++;
	k_work_submit(&led_work);
}

static int configure_led(void)
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
	if (configure_led() < 0) {
		return 0;
	}

	k_work_init(&led_work, led_work_handler);
	k_timer_init(&led_timer, led_timer_handler, NULL);
	k_timer_start(&led_timer, K_MSEC(TIMER_PERIOD_MS),
		      K_MSEC(TIMER_PERIOD_MS));

	printk("Lesson 10: timer and workqueue\n");
	printk("Timer period: %u ms\n", TIMER_PERIOD_MS);
	printk("Timer ISR submits work; work toggles blue LED and prints.\n");

	while (1) {
		k_sleep(K_FOREVER);
	}

	return 0;
}
