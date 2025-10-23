/*
 *  SPDX-License-Identifier: LGPL-2.1-or-later
 *
 *  Aether Xiegu X6100 Control
 *
 *  Copyright (c) 2022 Belousov Oleg aka R1CBU
 *  Copyright (c) 2022 Rui Oliveira aka CT7ALW
 */

#define _GNU_SOURCE
#include "aether_radio/x6100_control/low/flow.h"
#include "aether_radio/x6100_control/low/crc16.h"
#include "aether_radio/x6100_control/low/crc32.h"
#include "aether_radio/x6100_control/low/uart.h"

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <time.h>


#define BUF_SIZE (sizeof(x6100_flow_t) * 3)

static uint8_t *buf = NULL;
static uint8_t *buf_write = NULL;

static uint32_t prev_hkey = 0;

static const uint32_t magic = 0xAA5555AA;

bool x6100_flow_init()
{
    if (!x6100_uart_open_fd()) {
        return false;
    }
    buf = malloc(BUF_SIZE);
    buf_write = buf;

    return true;
}

AETHER_X6100CTRL_API bool x6100_flow_restart() {
    x6100_uart_close_fd();
    buf_write = buf;

    usleep(10000);

    return x6100_uart_open_fd();
}

static bool flow_check(x6100_flow_t *pack)
{
    uint32_t crc;
    uint32_t hkey;
    uint32_t pack_crc;
    uint8_t *begin;

    size_t hkey_offset = offsetof(x6100_flow_t, hkey);
    size_t crc_offset = offsetof(x6100_flow_t, crc);

    uint8_t* read_ptr = buf;

    bool result = false;
    while ((buf_write - read_ptr) >= sizeof(x6100_flow_t)) {
        begin = memmem(read_ptr, buf_write - read_ptr, &magic, sizeof(magic));

        if (begin == NULL) {
            read_ptr = buf_write - sizeof(magic);
            break;
        }
        if ((buf_write - begin) < sizeof(x6100_flow_t)) {
            read_ptr = begin;
            break;
        }

        hkey = *(uint32_t*)(begin + hkey_offset);
        pack_crc = *(uint32_t*)(begin + crc_offset);

        crc = calc_crc32((const uint32_t*)begin, sizeof(x6100_flow_t) / 4 - 1);

        if (pack_crc == crc) {
            result = true;
        } else {
            // Try use previous hkey
            *(uint32_t*)(begin + hkey_offset) = prev_hkey;
            crc = calc_crc32((const uint32_t*)begin, sizeof(x6100_flow_t) / 4 - 1);
            if (pack_crc == crc) {
                result = true;
            }
        }

        if (!result) {
            read_ptr = begin + 3;

        } else {
            memcpy((void *) pack, (void *) begin, sizeof(x6100_flow_t));
            pack->hkey = hkey;
            read_ptr = begin + sizeof(x6100_flow_t);
            prev_hkey = hkey;
            break;
        }
    }

    if (read_ptr < buf_write) {
        uint16_t tail_len = buf_write - read_ptr;
        memmove(buf, read_ptr, tail_len);
        buf_write = buf + tail_len;
    }
    return result;
}

bool x6100_flow_read(x6100_flow_t *pack)
{
    size_t buf_space = buf + BUF_SIZE - buf_write;
    size_t buf_used = buf_write - buf;
    if (buf_space < sizeof(x6100_flow_t)) {
        size_t shift = sizeof(x6100_flow_t) - buf_space;
        memmove(buf, buf + shift, buf_used - shift);
        buf_write -= shift;
    }

    int res = uart_read(buf_write, sizeof(x6100_flow_t));

    if (res > 0) {
        buf_write += res;

        if ((buf_write - buf) > sizeof(x6100_flow_t)) {
            return flow_check(pack);
        }
    }

    return false;
}
