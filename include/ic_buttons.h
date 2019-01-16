/**!
 * @file ic_buttons.h
 * @author Geymerson Ramos (geymerson@ic.ufal.br)
 * @brief
 * @version 0.1
 * @date 2018-12-15
 *
 * @copyright Copyright (c) IC 2018
 *
 */

#ifndef _IC_BUTTONS_H_
#define _IC_BUTTONS_H_

#include <device.h>
#include <gpio.h>
#include <logging/sys_log.h>
#include <misc/util.h>
#include <zephyr/types.h>


/* change to use another GPIO pin interrupt config */
#ifdef SW0_GPIO_FLAGS
#define EDGE (SW0_GPIO_FLAGS | GPIO_INT_EDGE)
#else
/*
 * If SW0_GPIO_FLAGS not defined used default EDGE value.
 * Change this to use a different interrupt trigger
 */
#define EDGE (GPIO_INT_EDGE | GPIO_INT_ACTIVE_LOW)
#endif


/* change this to enable pull-up/pull-down */
#ifdef SW0_GPIO_PIN_PUD
#define PULL_UP SW0_GPIO_PIN_PUD
#else
#define PULL_UP 0
#endif

// Buttons GPIO communication PORT
#define BUTTONS_PORT SW0_GPIO_CONTROLLER

// Defining buttons pins
#define BUTTON0 SW0_GPIO_PIN
#define BUTTON1 SW1_GPIO_PIN
#define BUTTON2 SW2_GPIO_PIN
#define BUTTON3 SW3_GPIO_PIN

#define BUTTON_NUMBERS 4
#define BUTTON_DEBOUNCE_DELAY_MS 250

// Buttons device struct
typedef struct {
    struct device *ic_device;
    struct gpio_callback ic_device_cb;
    u8_t initiated;
} ic_buttons_device_t;

extern u32_t time;
extern u32_t last_time;

extern ic_buttons_device_t buttons_dev;
extern struct k_work pressed_work[BUTTON_NUMBERS];

extern void button1_handler(struct k_work *work);
extern void button2_handler(struct k_work *work);
extern void button3_handler(struct k_work *work);
extern void button4_handler(struct k_work *work);


/**
 * @brief initiate buttons device
 *
 * This function configures the buttons' device communication port.
 * If the device was not initialized, no other function can be called
 *
 * @param buttons_device
 * @ret 0 if the communication was successffuly initialized, otherwise return an error
 */
int ic_buttons_init_device(ic_buttons_device_t *buttons_device);


/**
 * @brief configures all the buttons pins
 *
 * This function configures the buttons' as input or output, active if HIGH or LOW, etc
 *
 * @param buttons_device
 */
int ic_buttons_configure(ic_buttons_device_t *buttons_device);


/**
 * @brief configures callback
 *
 * This function is used to initiate, add and enable callbacks
 *
 * @param buttons_device
 * @param callback_function
 */
int ic_buttons_configure_callback(ic_buttons_device_t *buttons_device,
                                  gpio_callback_handler_t callback_function);


extern void ic_buttons_callback(struct device *buttons_device, struct gpio_callback *callback,
                                u32_t button_pin_mask);

uint8_t ic_buttons_pin_to_i(uint32_t pin_pos);

#endif  // _IC_BUTTONS_H_
