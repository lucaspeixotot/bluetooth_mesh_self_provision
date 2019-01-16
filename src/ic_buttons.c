/**
 * @file ic_buttons.c
 * @Lucas Peixoto (lpdac@ic.ufal.br)
 * @brief
 * @version 0.1
 * @date 2018-12-17
 *
 * @copyright Copyright (c) 2018
 *
 */
#include "ic_buttons.h"

int ic_buttons_init_device(ic_buttons_device_t *buttons_device)
{
    if (buttons_device->initiated != 0) {
        return -EBUSY;
    }
    struct device *buttons_dev;
    struct gpio_callback gpio_cb;
    buttons_dev                  = device_get_binding(BUTTONS_PORT);
    buttons_device->ic_device    = buttons_dev;
    buttons_device->ic_device_cb = gpio_cb;
    buttons_device->initiated    = 1;
    return 0;
}

int ic_buttons_configure(ic_buttons_device_t *buttons_device)
{
    if (buttons_device->initiated != 1) {
        return -ENODEV;
    }

    gpio_pin_configure(buttons_device->ic_device, BUTTON0, GPIO_DIR_IN | GPIO_INT | PULL_UP | EDGE);

    gpio_pin_configure(buttons_device->ic_device, BUTTON1, GPIO_DIR_IN | GPIO_INT | PULL_UP | EDGE);

    gpio_pin_configure(buttons_device->ic_device, BUTTON2, GPIO_DIR_IN | GPIO_INT | PULL_UP | EDGE);

    gpio_pin_configure(buttons_device->ic_device, BUTTON3, GPIO_DIR_IN | GPIO_INT | PULL_UP | EDGE);


    return 0;
}

int ic_buttons_configure_callback(ic_buttons_device_t *buttons_device,
                                  gpio_callback_handler_t callback_function)
{
    if (buttons_device->initiated != 1) {
        return -ENODEV;
    }
    gpio_init_callback(&buttons_device->ic_device_cb, callback_function,
                       BIT(BUTTON0) | BIT(BUTTON1) | BIT(BUTTON2) | BIT(BUTTON3));
    gpio_add_callback(buttons_device->ic_device, &buttons_device->ic_device_cb);
    gpio_pin_enable_callback(buttons_device->ic_device, BUTTON0);
    gpio_pin_enable_callback(buttons_device->ic_device, BUTTON1);
    gpio_pin_enable_callback(buttons_device->ic_device, BUTTON2);
    gpio_pin_enable_callback(buttons_device->ic_device, BUTTON3);
    return 0;
}


uint8_t ic_buttons_pin_to_i(uint32_t pin_pos)
{
    switch (pin_pos) {
    case BIT(BUTTON0):
        return 0;
    case BIT(BUTTON1):
        return 1;
    case BIT(BUTTON2):
        return 2;
    case BIT(BUTTON3):
        return 3;
    default:
        printk("No match for GPIO pin 0x%08x\n", pin_pos);
        return -EINVAL;
    }
    return 0;
}