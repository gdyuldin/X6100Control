/*
 *  SPDX-License-Identifier: LGPL-2.1-or-later
 *
 *  Aether Xiegu X6100 Control
 *
 *  Copyright (c) 2025 Georgy Dyuldin aka R2RFE
 */

#include "aether_radio/x6100_control/low/uart.h"

#include <fcntl.h>
#include <stdio.h>
#include <termios.h>
#include <pthread.h>
#include <errno.h>



static int uart_fd;
static pthread_mutex_t fd_mutex = PTHREAD_MUTEX_INITIALIZER;


bool x6100_uart_open_fd() {
    if (uart_fd > 0) {
        printf("UART already opened\n");
        return true;
    }
    uart_fd = open("/dev/ttyS1", O_RDWR | O_NONBLOCK | O_NOCTTY);

    if (uart_fd < 0)
        return false;

    struct termios attr;

    tcgetattr(uart_fd, &attr);

    cfsetispeed(&attr, B1152000);
    cfsetospeed(&attr, B1152000);

#if 1
    cfmakeraw(&attr);
#else
    attr.c_cflag = attr.c_cflag & 0xfffffe8f | 0x30;
    attr.c_iflag = attr.c_iflag & 0xfffffa14;
    attr.c_oflag = attr.c_oflag & 0xfffffffa;
    attr.c_lflag = attr.c_lflag & 0xffff7fb4;
#endif

    if (tcsetattr(uart_fd, 0, &attr) < 0)
    {
        x6100_uart_close_fd(uart_fd);
        return false;
    }
    return true;
}


bool x6100_uart_close_fd() {
    if (uart_fd > 0) {
        close(uart_fd);
        uart_fd = 0;
    } else {
        printf("UART already closed\n");
    }
    return true;
}

void uart_lock()
{
    pthread_mutex_lock(&fd_mutex);
}

void uart_unlock()
{
    pthread_mutex_unlock(&fd_mutex);
}


ssize_t uart_read(void *__buf, size_t __nbytes)
{
    uart_lock();
    ssize_t res = read(uart_fd, __buf, __nbytes);
    uart_unlock();
    return res;
}

void uart_flush()
{
    int res = tcflush(uart_fd, TCIOFLUSH);
    if (res != 0) {
        perror("Can't flush UART");
    }
}

bool uart_communicate(char *data, size_t data_len, char *answer, size_t answer_len,
                      size_t wait_for_ms)
{
    if (!uart_fd) {
        printf("Flow fd is not opened\n");
        return false;
    }

    while (data_len) {
        ssize_t size = write(uart_fd, data, data_len);
        if (size < 0) {
            if (errno == EAGAIN) {
                usleep(1000);
                continue;
            }
            perror("Error during send data\n");
            return false;
        }
        data_len -= size;
        data += size;
    }

    char *buf = answer;
    for (size_t i = 0; i < wait_for_ms; i++) {
        ssize_t size = read(uart_fd, buf, answer_len);
        if (size < 0) {
            if (errno == EAGAIN) {
                usleep(1000);
                continue;
            }
            perror("Error during receiving data");
            return false;
        }

        answer_len -= size;
        buf += size;
        if (answer_len <= 0) {
            break;
        } else {
            usleep(1000);
        }
    }
    if (answer_len > 0) {
        printf("Waiting answer timeout\n");
        return false;
    } else {
        return true;
    }
}
