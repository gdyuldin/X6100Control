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

AETHER_X6100CTRL_NO_EXPORT uint32_t calc_crc32(const uint32_t *data, uint16_t len);
