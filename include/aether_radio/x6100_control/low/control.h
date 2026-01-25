/*
 *  SPDX-License-Identifier: LGPL-2.1-or-later
 *
 *  Aether Xiegu X6100 Control
 *
 *  Copyright (c) 2022 Belousov Oleg a.k.a. R1CBU
 *  Copyright (c) 2022 Rui Oliveira a.k.a. CT7ALW
 */

#pragma once

#include "aether_radio/x6100_control/api.h"

#include <stdbool.h>
#include <stdint.h>

typedef enum
{
    x6100_vfoa_ham_band = 0,
    x6100_vfoa_freq,
    x6100_vfoa_att,
    x6100_vfoa_pre,
    x6100_vfoa_mode,
    x6100_vfoa_agc,

    x6100_vfob_ham_band = 6,
    x6100_vfob_freq,
    x6100_vfob_att,
    x6100_vfob_pre,
    x6100_vfob_mode,
    x6100_vfob_agc,

    x6100_sple_atue_trx = 12,
    x6100_vi_vm,
    x6100_rxvol,  // free 3 bytes
    x6100_rfg_txpwr, // plus fft_span

    x6100_reg_16 = 16,  // last 2 bytes - 2 parameters?
    x6100_atu_network = 17,  // last 2 bytes and bool value

    x6100_flow_fm_emp = 18,
    x6100_dac_adc_offsets = 19,

    // gap - 2 registers

    x6100_ling_loutg_imicg_hmicg = 20,
    x6100_micsel_pttmode_chge_spmode_auxiqgen_sqlthr,
    x6100_voxg_voxag_voxdly_voxe,
    x6100_nrthr_nbw_nbthr_nre_nbe,
    x6100_dnfcnt_dnfwidth_dnfe,
    x6100_cmplevel_cmpe,

    x6100_agcknee_agcslope_agchang = 27,
    x6100_agctime,
    x6100_monilevel,

    x6100_ks_km_kimb_cwtone_cwvol_cwtrain = 33,
    x6100_qsktime_kr,
    x6100_if_shift,  // on/off and shift

    x6100_biasdrive_biasfinal = 41,
    x6100_rit,
    x6100_xit,
    x6100_filter1_low,
    x6100_filter1_high,
    x6100_filter2_low,
    x6100_filter2_high,

    x6100_rphscmp = 48, // RX PHS COMP, multiplied by 0.001 on BASE, looks like not used
    x6100_rmagcmp, // RX MAG COMP, multiplied by 0.0001 on BASE, looks like not used
    x6100_txiofs, // TXI DC OFFSET, multiplied by 5000 on BASE
    x6100_txqofs, // TXQ DC OFFSET, multiplied by 5000 on BASE

    x6100_pwrsync = 53,
    x6100_last = 55
} x6100_cmd_enum_t;

/* Regs x6100_vfoa... x6100_vfob... */

typedef enum
{
    x6100_agc_off = 0,
    x6100_agc_slow = 1,
    x6100_agc_fast = 2,
    x6100_agc_auto = 3
} x6100_agc_t;

typedef enum
{
    x6100_att_off = 0,
    x6100_att_on = 1
} x6100_att_t;

typedef enum
{
    x6100_mode_lsb = 0,
    x6100_mode_lsb_dig = 1,
    x6100_mode_usb = 2,
    x6100_mode_usb_dig = 3,
    x6100_mode_cw = 4,
    x6100_mode_cwr = 5,
    x6100_mode_am = 6,
    x6100_mode_nfm = 7
} x6100_mode_t;

typedef enum
{
    x6100_pre_off = 0,
    x6100_pre_on = 1
} x6100_pre_t;

/* Reg x6100_sple_atue_trx */

enum
{
    x6100_sple = 0x00002,
    x6100_voice_rec = 0x00008,
    x6100_swrscan_trx = 0x00010,
    x6100_atue = 0x01000,
    x6100_atu_tune = 0x02000,
    x6100_modem_trx = 0x04000,
    x6100_calibration_trx = 0x08000,
    x6100_power_off = 0x10000,
    x6100_iptt = 0x40000
};

typedef enum
{
    x6100_key_manual = 0,
    x6100_key_auto_left = 1,
    x6100_key_auto_right = 2
} x6100_key_mode_t;

typedef enum
{
    x6100_iambic_a = 0,
    x6100_iambic_b = 1
} x6100_iambic_mode_t;

typedef enum {
    x6100_mic_builtin = 0,
    x6100_mic_handle,
    x6100_mic_auto
} x6100_mic_sel_t;

typedef enum {
    x6100_comp_off = 0,
    x6100_comp_1_2,
    x6100_comp_1_4,
    x6100_comp_1_8
} x6100_comp_level_t;


typedef struct {
    uint8_t major;
    uint8_t minor;
    uint8_t patch;
    uint8_t rev;
} x6100_base_ver_t;

typedef union {
    uint32_t i;
    struct {
        uint8_t flow_fp16: 1;
        uint8_t fm_emp: 1;
    } v;
} x6100_reg_flow_fm_emp_flags_t;

typedef union {
    uint32_t i;
    struct {
        uint16_t adc_dac_gain_offset;  // bf16
        uint16_t dac_gain_offset;  // bf16
    } v;
} x6100_reg_dac_adc_offsets_t;

/* Functions */

AETHER_X6100CTRL_API bool x6100_control_init();
AETHER_X6100CTRL_API bool x6100_control_cmd(x6100_cmd_enum_t cmd, uint32_t arg);
AETHER_X6100CTRL_API void x6100_control_idle();
AETHER_X6100CTRL_API bool x6100_control_set_band(uint32_t freq);
AETHER_X6100CTRL_API uint32_t x6100_control_get(x6100_cmd_enum_t cmd);
AETHER_X6100CTRL_API char* x6100_control_get_fw_version_str();
AETHER_X6100CTRL_API x6100_base_ver_t x6100_control_get_base_ver();

AETHER_X6100CTRL_NO_EXPORT void i2c_lock();
AETHER_X6100CTRL_NO_EXPORT void i2c_unlock();
