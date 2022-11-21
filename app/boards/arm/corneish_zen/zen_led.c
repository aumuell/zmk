/*
 * Copyright (c) 2016 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr.h>
#include <device.h>
#include <devicetree.h>
#include <drivers/gpio.h>

#include <logging/log.h>

LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

//#define LED_NODE DT_ALIAS(led_0)
#define LED_NODE DT_PATH(leds, led_0)

#if DT_NODE_HAS_STATUS(LED_NODE, okay)
#define LED DT_GPIO_LABEL(LED_NODE, gpios)
#define PIN DT_GPIO_PIN(LED_NODE, gpios)
#define FLAGS DT_GPIO_FLAGS(LED_NODE, gpios)
#else
/* A build error here means your board isn't set up to blink an LED. */
#error "Unsupported board: led devicetree alias is not defined"
#define LED0	""
#define PIN	0
#define FLAGS	0
#endif

static const struct device *pwr_led_dev = NULL;

int led_set_state(bool state) {

    if (!pwr_led_dev) {
        LOG_ERR("LED not initialized");
        return -EIO;
    }

    if (state)
        LOG_DBG("Setting LED pin to low for on");
    else
        LOG_DBG("Setting LED pin to high for off");

    return gpio_pin_set(pwr_led_dev, PIN, state); // set to false so no constant battery drain, just testing for now
}

static int pwr_led_init(const struct device *dev) {

	int ret;

	dev = device_get_binding(LED);
	if (dev == NULL) {
            LOG_ERR("No LED binding");
		return -EIO;
	}

        pwr_led_dev = dev;

	ret = gpio_pin_configure(dev, PIN, GPIO_OUTPUT | FLAGS);
	if (ret < 0) {
            LOG_ERR("Failed to configure LED pin: %d", ret);
		return -EIO;
	}

        return led_set_state(false);
}


SYS_INIT(pwr_led_init, APPLICATION, CONFIG_APPLICATION_INIT_PRIORITY);
