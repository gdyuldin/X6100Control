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

enum x6100_fw_upgrade_code {
    X6100_FW_UPGRADE_SENDING,
    X6100_FW_UPGRADE_DONE,
    X6100_FW_UPGRADE_ERR_START=100,
    X6100_FW_UPGRADE_FILE_ERROR,
    X6100_FW_UPGRADE_RESET_ERROR,
    X6100_FW_UPGRADE_ERASE_FLASH_ERROR,
    X6100_FW_UPGRADE_START_XMODEM_ERROR,
    X6100_FW_UPGRADE_SENDING_ERROR,
    X6100_FW_UPGRADE_REBOOT_ERROR,
};

typedef struct {
    enum x6100_fw_upgrade_code code;
    int progress;
} x6100_fw_upgrade_msg_t;

typedef void (*x6100_fw_upgrade_cb)(x6100_fw_upgrade_msg_t msg);

AETHER_X6100CTRL_API bool x6100_base_firmware_upgrade(const char *path, x6100_fw_upgrade_cb notify_cb);
