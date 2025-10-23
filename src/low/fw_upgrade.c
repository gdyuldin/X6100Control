/*
 *  SPDX-License-Identifier: LGPL-2.1-or-later
 *
 *  Aether Xiegu X6100 Control
 *
 *  Copyright (c) 2025 Georgy Dyuldin aka R2RFE
 */

#include "aether_radio/x6100_control/low/fw_upgrade.h"

#include "aether_radio/x6100_control/low/uart.h"
#include "aether_radio/x6100_control/low/control.h"
#include "aether_radio/x6100_control/low/gpio.h"
#include "aether_radio/x6100_control/low/crc16.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define CHUNK_SIZE 1024


static bool send_file(const char *data, size_t data_size, x6100_fw_upgrade_cb notify_cb)
{
    x6100_fw_upgrade_msg_t msg;
    float progress;

    struct  __attribute__((__packed__))
    {
        char start;
        char n;
        char n_inv;
        char data[CHUNK_SIZE];
        uint16_t crc;
    } chunk;
    chunk.start = 2;
    printf("Sending file: %d with %d (%d)\n", data_size, CHUNK_SIZE, data_size/CHUNK_SIZE);
    for (int i = 0; i <= (data_size / CHUNK_SIZE); i++)
    {
        progress = (float)i * 100.0f * CHUNK_SIZE / data_size;
        printf("\r%0.1f", progress);
        fflush(stdout);
        chunk.n = (char)(i + 1);
        chunk.n_inv = ~chunk.n;
        memcpy(chunk.data, data + i * CHUNK_SIZE, CHUNK_SIZE);
        uint16_t crc = crc16(chunk.data, CHUNK_SIZE);
        chunk.crc = (crc >> 8) | (crc << 8);

        char response;
        bool res = uart_communicate((char *)&chunk, sizeof(chunk), &response, 1, 30);
        if (!res)
        {
            printf("Can't write firmware chunk\n");
            msg.code = X6100_FW_UPGRADE_SENDING_ERROR;
            notify_cb(msg);
            return false;
        }
        switch (response)
        {
        case 0x18:
            printf("Base firmware update canceled\n");
            msg.code = X6100_FW_UPGRADE_SENDING_ERROR;
            notify_cb(msg);
            return false;
            break;
        case 0x15:
            printf("Bad packet\n");
            msg.code = X6100_FW_UPGRADE_SENDING_ERROR;
            notify_cb(msg);
            return false;
            break;
        case 0x06:
            break;
        default:
            printf("Error processing base firmware chunk: %#04x\n", response);
            msg.code = X6100_FW_UPGRADE_SENDING_ERROR;
            notify_cb(msg);
            return false;
            break;
        }
        msg.code = X6100_FW_UPGRADE_SENDING;
        msg.progress = progress;
        notify_cb(msg);
    }
    printf("\nDone\n");
    return true;
}

static bool fw_upgrade_helper(const char *data, size_t data_size, x6100_fw_upgrade_cb notify_cb) {
    char cmd;
    char answer[2];
    bool res;
    x6100_fw_upgrade_msg_t msg;

    // x6100_gpio_set(x6100_pin_morse_key, 1);
    // x6100_gpio_set(x6100_pin_bb_reset, 0);

    // printf("a\n");
    // usleep(1000000);
    // printf("b\n");

    printf("Flush UART buffers\n");
    uart_flush();

    printf("Send reset\n");
    x6100_gpio_set(x6100_pin_morse_key, 0);
    x6100_gpio_set(x6100_pin_bb_reset, 1);
    usleep(100000);

    printf("Flush UART buffers\n");
    uart_flush();

    printf("Release bb reset\n");
    x6100_gpio_set(x6100_pin_bb_reset, 0);
    usleep(1000000);

    bool iap_result = false;
    for (size_t i = 0; i < 51; i++)
    {
        res = uart_communicate(NULL, 0, answer, 1, 20000);
        if (!res) {
            printf("IAP timeout\n");
            usleep(100000);
            continue;
        }
        if ((answer[0] & 0xdf) == 0x41) {
            printf("Start IAP ok\n");
            iap_result = true;
            break;
        } else {
            printf("Wrong ack\n");
        }
    }

    printf("Release morse key\n");
    x6100_gpio_set(x6100_pin_morse_key, 1);

    if (!iap_result) {
        msg.code = X6100_FW_UPGRADE_RESET_ERROR;
        notify_cb(msg);
        return false;
    }

    // erase flash
    printf("Erase flash\n");
    cmd = 0x72;
    res = uart_communicate(&cmd, 1, answer, 1, 60000);
    if (!res) {
        printf("Can't erase flash\n");
        msg.code = X6100_FW_UPGRADE_ERASE_FLASH_ERROR;
        notify_cb(msg);
        return false;
    }
    if (answer[0] != 0x67) {
        printf("Unexpected answer after erasing flash: %#04x\n", answer[0]);
        msg.code = X6100_FW_UPGRADE_ERASE_FLASH_ERROR;
        notify_cb(msg);
        return false;
    }

    // start xmodem
    printf("Start xmodem\n");
    cmd = 0x70;
    res = uart_communicate(&cmd, 1, answer, 1, 1000);
    if (!res) {
        printf("Can't start xmodem\n");
        msg.code = X6100_FW_UPGRADE_START_XMODEM_ERROR;
        notify_cb(msg);
        return false;
    }
    if (answer[0] != 0x67) {
        printf("Unexpected answer after start xmodem command flash: %#04x\n", answer[0]);
        msg.code = X6100_FW_UPGRADE_START_XMODEM_ERROR;
        notify_cb(msg);
        return false;
    }

    // Wait for xmodem start
    res = uart_communicate(NULL, 0, answer, 2, 1000);
    if (!res) {
        printf("Error waiting flash state\n");
        msg.code = X6100_FW_UPGRADE_START_XMODEM_ERROR;
        notify_cb(msg);
        return false;
    }
    if ((answer[1] & 0xdf) != 0x43) {
        printf("Unexpected xmodem answer: %#04x\n", answer[1]);
        msg.code = X6100_FW_UPGRADE_START_XMODEM_ERROR;
        notify_cb(msg);
        return false;
    }

    // send file
    res = send_file(data, data_size, notify_cb);
    if (!res) {
        printf("Error during flashing base firmware\n");
        return false;
    }

    // send reboot
    cmd = 0x04;
    res = uart_communicate(&cmd, 1, answer, 1, 10);
    if (!res) {
        printf("Error during reboot after firmware update\n");
        msg.code = X6100_FW_UPGRADE_REBOOT_ERROR;
        notify_cb(msg);
        return false;
    }
    if (answer[0] != 0x06) {
        printf("Unexpected answer from BASE for reboot after upgrade: %#04x\n", answer[0]);
        msg.code = X6100_FW_UPGRADE_REBOOT_ERROR;
        notify_cb(msg);
        return false;
    }
    usleep(5000000);
    // Wait for boot
    res = uart_communicate(NULL, 0, answer, 1, 5000000);

    // Refresh i2c registers
    x6100_control_idle();
    return true;
}

bool x6100_base_firmware_upgrade(const char *path, x6100_fw_upgrade_cb notify_cb) {

    x6100_fw_upgrade_msg_t msg;
    FILE *f = fopen(path, "rb");
    if (!f) {
        perror("Can't open firmware file");
        msg.code = X6100_FW_UPGRADE_FILE_ERROR;
        notify_cb(msg);
        return false;
    }
    // Read file and pad to 1024 bytes with 0xff
    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    fseek(f, 0, SEEK_SET);

    size_t pad_size = -(size_t)fsize % CHUNK_SIZE;
    char *data = malloc(fsize + pad_size);
    memset(data + fsize, 0xff, pad_size);
    size_t n_chunks = fread(data, fsize, 1, f);
    fclose(f);
    if (n_chunks < 1) {
        printf("Error during read firmware file\n");
        msg.code = X6100_FW_UPGRADE_FILE_ERROR;
        notify_cb(msg);
        return false;
    }

    uart_lock();
    i2c_lock();
    bool res = fw_upgrade_helper(data, fsize + pad_size, notify_cb);
    i2c_unlock();
    uart_unlock();
    msg.code = X6100_FW_UPGRADE_REBOOT_ERROR;
    notify_cb(msg);
    return res;
}
