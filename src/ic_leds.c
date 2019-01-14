/**
 * @file ic_leds.c
 * @Lucas Peixoto(lpdac@ic.ufal.br)
 * @brief
 * @version 0.1
 * @date 2018-12-17
 *
 * @copyright Copyright (c) 2018
 *
 */
#include "ic_leds.h"


int ic_leds_init_device(ic_leds_device_t *leds_device)
{
    if (leds_device->initiated != 0) {
        return -EBUSY;
    }
    struct device *led_dev;
    led_dev                = device_get_binding(LED_PORT);
    leds_device->ic_device = led_dev;
    leds_device->initiated = 1;
    return 0;
}

int ic_leds_configure(ic_leds_device_t *leds_device)
{
    if (leds_device->initiated != 1) {
        return -ENODEV;
    }

    gpio_pin_configure(leds_device->ic_device, LED0,
                       GPIO_DIR_OUT | GPIO_PUD_PULL_UP | GPIO_INT_ACTIVE_HIGH);

    gpio_pin_configure(leds_device->ic_device, LED1,
                       GPIO_DIR_OUT | GPIO_PUD_PULL_UP | GPIO_INT_ACTIVE_HIGH);

    gpio_pin_configure(leds_device->ic_device, LED2,
                       GPIO_DIR_OUT | GPIO_PUD_PULL_UP | GPIO_INT_ACTIVE_HIGH);

    gpio_pin_configure(leds_device->ic_device, LED3,
                       GPIO_DIR_OUT | GPIO_PUD_PULL_UP | GPIO_INT_ACTIVE_HIGH);


    return 0;
}

int ic_leds_valid_pin_mask(u32_t led_pin_mask)
{
    if (led_pin_mask == LED0 || led_pin_mask == LED1 || led_pin_mask == LED2
        || led_pin_mask == LED3) {
        return 1;
    }
    return 0;
}


int ic_leds_turn_all_leds_on(ic_leds_device_t *leds_device)
{
    if (leds_device->initiated != 1) {
        return -ENODEV;
    }
    gpio_pin_write(leds_device->ic_device, LED0, HIGH);
    leds_states[0] = HIGH;
    gpio_pin_write(leds_device->ic_device, LED1, HIGH);
    leds_states[1] = HIGH;
    gpio_pin_write(leds_device->ic_device, LED2, HIGH);
    leds_states[2] = HIGH;
    gpio_pin_write(leds_device->ic_device, LED3, HIGH);
    leds_states[3] = HIGH;
    return 0;
}

int ic_leds_turn_all_leds_off(ic_leds_device_t *leds_device)
{
    if (leds_device->initiated != 1) {
        return -ENODEV;
    }

    gpio_pin_write(leds_device->ic_device, LED0, LOW);
    leds_states[0] = LOW;
    gpio_pin_write(leds_device->ic_device, LED1, LOW);
    leds_states[1] = LOW;
    gpio_pin_write(leds_device->ic_device, LED2, LOW);
    leds_states[2] = LOW;
    gpio_pin_write(leds_device->ic_device, LED3, LOW);
    leds_states[3] = LOW;
    return 0;
}

int ic_leds_turn_on_led(ic_leds_device_t *leds_device, u32_t led_pin_mask)
{
    if (leds_device->initiated != 1) {
        return -ENODEV;
    }
    if (led_pin_mask == LED0) {
        leds_states[0] = HIGH;
    } else if (led_pin_mask == LED1) {
        leds_states[1] = HIGH;
    } else if (led_pin_mask == LED2) {
        leds_states[2] = HIGH;
    } else if (led_pin_mask == LED3) {
        leds_states[3] = HIGH;
    } else {
        return -ENOTSUP;
    }
    gpio_pin_write(leds_device->ic_device, led_pin_mask, HIGH);
    return 0;
}

int ic_leds_turn_off_led(ic_leds_device_t *leds_device, u32_t led_pin_mask)
{
    if (leds_device->initiated != 1) {
        return -ENODEV;
    }
    if (led_pin_mask == LED0) {
        leds_states[0] = LOW;
    } else if (led_pin_mask == LED1) {
        leds_states[1] = LOW;
    } else if (led_pin_mask == LED2) {
        leds_states[2] = LOW;
    } else if (led_pin_mask == LED3) {
        leds_states[3] = LOW;
    } else {
        return -ENOTSUP;
    }
    gpio_pin_write(leds_device->ic_device, led_pin_mask, LOW);
    return 0;
}
