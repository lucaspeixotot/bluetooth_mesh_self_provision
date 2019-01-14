/**!
 * @file ic_leds.h
 * @author Geymerson Ramos (geymerson@ic.ufal.br)
 * @brief
 * @version 0.1
 * @date 2018-12-15
 *
 * @copyright Copyright (c) IC 2018
 *
 */
#ifndef _IC_LEDS_H_
#define _IC_LEDS_H_

#include <device.h>
#include <gpio.h>
#include <logging/sys_log.h>
#include <zephyr/types.h>

#define LOW 1
#define HIGH 0

// Configure LED GPIO communication port
#define LED_PORT LED0_GPIO_CONTROLLER

// Configure LED GPIO PIN
#define LED0 LED0_GPIO_PIN
#define LED1 LED1_GPIO_PIN
#define LED2 LED2_GPIO_PIN
#define LED3 LED3_GPIO_PIN


typedef struct {
    struct device *ic_device;
    u8_t initiated;
} ic_leds_device_t;

extern u8_t leds_states[4];
extern ic_leds_device_t leds_dev;

/**
 * @brief initiate buttons device
 *
 * This function configures the leds' device communication port.
 * If the device was not initialized, no other function can be called
 *
 * @param leds_device
 * @ret 0 if the communication was successffuly initialized, otherwise return an error
 */
int ic_leds_init_device(ic_leds_device_t *leds_device);

/**
 * @brief Checks if led_pin_mask is a valid led pin number
 *
 * @param leds_device
 * @ret 1 if the led_pin_mask is a valid value, otherwise return 0
 */
int ic_leds_valid_pin_mask(u32_t led_pin_mask);


/**
 * @brief configures all the leds pins
 *
 * This function configures the leds' as input or output, active if HIGH or LOW, etc
 *
 * @param leds_device
 */
int ic_leds_configure(ic_leds_device_t *leds_device);

/**
 * @brief Turn on all the leds
 *
 * @param leds_device
 */
int ic_leds_turn_all_leds_on(ic_leds_device_t *leds_device);

/**
 * @brief Turn off all the leds
 *
 * @param leds_device
 */


int ic_leds_turn_all_leds_off(ic_leds_device_t *leds_device);

/**
 * @brief Turn on the specific led with pin address value = led_pin_mask
 *
 * @param leds_device
 * @param led_pin_mask
 */
int ic_leds_turn_on_led(ic_leds_device_t *leds_device, u32_t led_pin_mask);

/**
 * @brief Turn off the specific led with pin address value = led_pin_mask
 *
 * @param leds_device
 * @param led_pin_mask
 */
int ic_leds_turn_off_led(ic_leds_device_t *leds_device, u32_t led_pin_mask);

#endif  // _IC_LEDS_H_
