/*
 *  SPDX-License-Identifier: LGPL-2.1-or-later
 *
 *  Aether Xiegu X6100 Control
 *
 *  Copyright (c) 2025 Georgy Dyuldin Oleg aka R2RFE
 */

#pragma once

#include "aether_radio/x6100_control/api.h"

#include <stdint.h>
#include <stddef.h>

AETHER_X6100CTRL_NO_EXPORT uint16_t crc16(const void *buf, size_t len);
