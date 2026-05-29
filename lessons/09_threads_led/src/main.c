/*
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <stdint.h>

#include <zephyr/drivers/gpio.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>

#define RED_NODE DT_ALIAS(led0)
#define GREEN_NODE DT_ALIAS(led1)
#define BLUE_NODE DT_ALIAS(led2)

#if !DT_NODE_HAS_STATUS(RED_NODE, okay) || \
	!DT_NODE_HAS_STATUS(GREEN_NODE, okay) || \
	!DT_NODE_HAS_STATUS(BLUE_NODE, okay)
#error "Unsupported board: led0, led1 and led2 aliases are required"
#endif

#define LED_STACK_SIZE 512
#define LED_THREAD_PRIORITY 7

struct led_thread_config {
	const char *name;
	const struct gpio_dt_spec led;
	uint32_t interval_ms;
};

static const struct led_thread_config red_cfg = {
	.name = "red",
	.led = GPIO_DT_SPEC_GET(RED_NODE, gpios),
	.interval_ms = 250U,
};

static const struct led_thread_config green_cfg = {
	.name = "green",
	.led = GPIO_DT_SPEC_GET(GREEN_NODE, gpios),
	.interval_ms = 500U,
};

static const struct led_thread_config blue_cfg = {
	.name = "blue",
	.led = GPIO_DT_SPEC_GET(BLUE_NODE, gpios),
	.interval_ms = 1000U,
};

K_SEM_DEFINE(start_sem, 0, 3);

static int configure_led(const struct led_thread_config *cfg)
{
	int ret;

	if (!gpio_is_ready_dt(&cfg->led)) {
		printk("%s LED GPIO is not ready\n", cfg->name);
		return -ENODEV;
	}

	ret = gpio_pin_configure_dt(&cfg->led, GPIO_OUTPUT_INACTIVE);
	if (ret < 0) {
		printk("Failed to configure %s LED: %d\n", cfg->name, ret);
		return ret;
	}

	return 0;
}

static void led_thread(void *arg1, void *arg2, void *arg3)
{
	const struct led_thread_config *cfg = arg1;
	uint32_t count = 0U;

	ARG_UNUSED(arg2);
	ARG_UNUSED(arg3);

	k_sem_take(&start_sem, K_FOREVER);
	printk("%s LED thread started, interval %u ms\n",
	       cfg->name, (unsigned int)cfg->interval_ms);

	while (1) {
		(void)gpio_pin_toggle_dt(&cfg->led);
		count++;
		printk("%s thread toggle %u\n",
		       cfg->name, (unsigned int)count);
		k_sleep(K_MSEC(cfg->interval_ms));
	}
}

K_THREAD_DEFINE(red_tid, LED_STACK_SIZE, led_thread,
		(void *)&red_cfg, NULL, NULL,
		LED_THREAD_PRIORITY, 0, 0);
K_THREAD_DEFINE(green_tid, LED_STACK_SIZE, led_thread,
		(void *)&green_cfg, NULL, NULL,
		LED_THREAD_PRIORITY, 0, 0);
K_THREAD_DEFINE(blue_tid, LED_STACK_SIZE, led_thread,
		(void *)&blue_cfg, NULL, NULL,
		LED_THREAD_PRIORITY, 0, 0);

int main(void)
{
	if (configure_led(&red_cfg) < 0 ||
	    configure_led(&green_cfg) < 0 ||
	    configure_led(&blue_cfg) < 0) {
		return 0;
	}

	printk("Lesson 09: threads LED\n");
	printk("Red 250 ms, green 500 ms, blue 1000 ms\n");

	k_sem_give(&start_sem);
	k_sem_give(&start_sem);
	k_sem_give(&start_sem);

	while (1) {
		k_sleep(K_FOREVER);
	}

	return 0;
}
