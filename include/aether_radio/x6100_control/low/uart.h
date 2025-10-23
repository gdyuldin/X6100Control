/*
 *  SPDX-License-Identifier: LGPL-2.1-or-later
 *
 *  Aether Xiegu X6100 Control
 *
 *  Copyright (c) 2025 Georgy Dyuldin Oleg aka R2RFE
 */

#pragma once

#include "aether_radio/x6100_control/api.h"

#include <stdbool.h>
#include <unistd.h>

AETHER_X6100CTRL_API bool x6100_uart_open_fd();
AETHER_X6100CTRL_API bool x6100_uart_close_fd();

AETHER_X6100CTRL_NO_EXPORT void uart_lock();
AETHER_X6100CTRL_NO_EXPORT void uart_unlock();

AETHER_X6100CTRL_NO_EXPORT ssize_t uart_read(void *__buf, size_t __nbytes);
AETHER_X6100CTRL_NO_EXPORT void uart_flush();

AETHER_X6100CTRL_NO_EXPORT bool uart_communicate(char *data, size_t data_len, char *answer, size_t answer_len, size_t wait_for_ms);
