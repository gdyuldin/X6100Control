/*
 *  SPDX-License-Identifier: LGPL-2.1-or-later
 *
 *  Aether Xiegu X6100 Control
 *
 *  Copyright (c) 2022 Belousov Oleg aka R1CBU
 *  Copyright (c) 2022 Rui Oliveira aka CT7ALW
 */

#include "aether_radio/x6100_control/low/gpio.h"

#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#include <linux/gpio.h>

#define DEV_NAME "/dev/gpiochip1"

		// pinctrl@1c20800 {
		// 	reg = <0x1c20800 0x400>;

static struct gpiohandle_data data;
static int fd;

int x6100_pin_wifi;
int x6100_pin_usb;
int x6100_pin_light;
int x6100_pin_morse_key;
int x6100_pin_bb_reset;

static bool gpio_open()
{
    fd = open(DEV_NAME, O_RDONLY);
    if (fd < 0)
    {
        printf("Unabled to open %s: %s\n", DEV_NAME, strerror(errno));
        return false;
    }
    printf("Open chip: %i\n", fd);
    struct gpiochip_info info;
    int ret = ioctl(fd, GPIO_GET_CHIPINFO_IOCTL, &info);
    printf("get chip info %i, lines: %u\n", ret, info.lines);
    return true;
}

static int gpio_pin_setup(unsigned int pin)
{
    struct gpio_v2_line_request req;
    int ret;

    memset(&req, 0, sizeof(req));
    // memset(&data.values, 0, sizeof(data.values));

    // request gpio output
    req.offsets[0] = pin;
    req.config.flags = GPIO_V2_LINE_FLAG_OUTPUT;
    req.config.num_attrs = 0;
    // req.flags = GPIOHANDLE_REQUEST_OUTPUT;
    strcpy(req.consumer, "X6100");
    req.num_lines = 1;
    // memcpy(req.default_values, &data, sizeof(req.default_values));

    ret = ioctl(fd, GPIO_V2_GET_LINE_IOCTL, &req);
    if (ret == -1) {
        fprintf(stderr, "Failed to issue %s (%d), %u, %s\n",
                "GPIO_V2_GET_LINE_IOCTL", ret, pin, strerror(errno));
        return ret;
    }
    return req.fd;
}

// static int gpio_open_value(uint16_t pin)
// {
//     char str[64];
//     int fd, len;

//     snprintf(str, sizeof(str), "/sys/class/gpio/gpio%i/value", pin);

//     return open(str, O_RDWR);
// }

// static bool gpio_create(uint16_t pin)
// {
//     char str[64];
//     int fd, len;

//     len = snprintf(str, sizeof(str), "%i\n", pin);
//     fd = open("/sys/class/gpio/export", O_WRONLY);

//     if (fd < 0)
//         return false;

//     if (write(fd, str, len) < 0)
//     {
//         close(fd);
//         return false;
//     }

//     close(fd);

//     snprintf(str, sizeof(str), "/sys/class/gpio/gpio%i/direction", pin);
//     fd = open(str, O_WRONLY);

//     if (fd < 0)
//         return false;

//     if (write(fd, "out\n", 4) < 0)
//     {
//         close(fd);
//         return false;
//     }

//     close(fd);
//     return true;
// }

// static int gpio_open(uint16_t pin)
// {
//     int ret;
//     ret = gpio_request_one(pin, GPIOF_OUT_INIT_LOW, "led1");
//     // int fd = gpio_open_value(pin);

//     // if (fd < 0)
//     // {
//     //     if (gpio_create(pin))
//     //     {
//     //         fd = gpio_open_value(pin);
//     //     }
//     //     else
//     //     {
//     //         fd = -1;
//     //     }
//     // }

//     // return fd;
// }

bool x6100_gpio_init()
{
    if (gpio_open()) {
        x6100_pin_wifi = 357;
        x6100_pin_usb = gpio_pin_setup(138);
        x6100_pin_light = gpio_pin_setup(143);
        x6100_pin_morse_key = gpio_pin_setup(203);
        x6100_pin_bb_reset = gpio_pin_setup(204);
        return true;
    }
    printf("Can't open gpiochip\n");
    return false;
    // return (x6100_pin_wifi > 0) && (x6100_pin_usb > 0) && (x6100_pin_light > 0) && (x6100_pin_morse_key > 0) && (x6100_pin_bb_reset > 0);
}

void x6100_gpio_set(int pin, int value)
{
    // printf("Set pin %i = %i, chip fd: %i\n", pin, value, fd);
    int ret;
    struct gpio_v2_line_values values;

    values.bits = value > 0;
    values.mask = 1;

    // set data
    ret = ioctl(pin, GPIO_V2_LINE_SET_VALUES_IOCTL, &values);
    if (ret == -1) {
        ret = -errno;
        fprintf(stderr, "Failed to issue %s (%d), %s\n",
                "GPIO_V2_LINE_SET_VALUES_IOCTL", ret, strerror(errno));
    }
}
