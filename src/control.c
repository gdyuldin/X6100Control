/*
 *  SPDX-License-Identifier: LGPL-2.1-or-later
 *
 *  Aether Xiegu X6100 Control
 *
 *  Copyright (c) 2022 Belousov Oleg aka R1CBU
 *  Copyright (c) 2022 Rui Oliveira aka CT7ALW
 */

#include <unistd.h>
#include <signal.h>
#include "aether_radio/x6100_control/control.h"

static x6100_vfo_t fg_vfo;

/* VFO Settings */

void x6100_control_vfo_mode_set(x6100_vfo_t vfo, x6100_mode_t mode)
{
    x6100_control_cmd(vfo == X6100_VFO_A ? x6100_vfoa_mode : x6100_vfob_mode, mode);
}

void x6100_control_vfo_freq_set(x6100_vfo_t vfo, uint32_t freq)
{
    if (vfo == fg_vfo)
    {
        if (x6100_control_set_band(freq)) {
            // Seems, BASE switches to VFO-A, when band was changed.
            // Force switch to active VFO.
            x6100_control_vfo_set(fg_vfo);
        }
    }

    if (vfo == X6100_VFO_A)
    {
        x6100_control_cmd(x6100_vfoa_freq, freq);
    }
    else
    {
        x6100_control_cmd(x6100_vfob_freq, freq);
    }
}

void x6100_control_vfo_agc_set(x6100_vfo_t vfo, x6100_agc_t agc)
{
    x6100_control_cmd(vfo == X6100_VFO_A ? x6100_vfoa_agc : x6100_vfob_agc, agc);
}

void x6100_control_vfo_att_set(x6100_vfo_t vfo, x6100_att_t att)
{
    x6100_control_cmd(vfo == X6100_VFO_A ? x6100_vfoa_att : x6100_vfob_att, att);
}

void x6100_control_vfo_pre_set(x6100_vfo_t vfo, x6100_pre_t pre)
{
    x6100_control_cmd(vfo == X6100_VFO_A ? x6100_vfoa_pre : x6100_vfob_pre, pre);
}

/* Sound settings */

void x6100_control_rxvol_set(uint8_t vol)
{
    x6100_control_cmd(x6100_rxvol, vol);
}

void x6100_control_record_set(bool on)
{
    uint32_t prev = x6100_control_get(x6100_sple_atue_trx);
    uint32_t next;

    if (on) {
        next = prev | x6100_voice_rec;
    } else {
        next = prev & (~x6100_voice_rec);
    }

    x6100_control_cmd(x6100_sple_atue_trx, next);
}

void x6100_control_spmode_set(bool phone)
{
    uint32_t prev = x6100_control_get(x6100_micsel_pttmode_chge_spmode_auxiqgen_sqlthr) & (~(1 << 5));

    x6100_control_cmd(x6100_micsel_pttmode_chge_spmode_auxiqgen_sqlthr, prev | (phone << 5));
}

/* Operation */

void x6100_control_ptt_set(bool on) {
    uint32_t prev = x6100_control_get(x6100_sple_atue_trx);
    uint32_t next;

    if (on) {
        next = prev | x6100_iptt;
    } else {
        next = prev & (~x6100_iptt);
    }

    x6100_control_cmd(x6100_sple_atue_trx, next);
}

void x6100_control_atu_tune(bool on)
{
    uint32_t prev = x6100_control_get(x6100_sple_atue_trx);
    uint32_t next;

    if (on) {
        next = prev | x6100_atu_tune;
    } else {
        next = prev & (~x6100_atu_tune);
    }

    x6100_control_cmd(x6100_sple_atue_trx, next);
}

void x6100_control_poweroff()
{
    uint32_t prev = x6100_control_get(x6100_sple_atue_trx);
    uint32_t next = prev | x6100_power_off;

    x6100_control_cmd(x6100_sple_atue_trx, next);
    sleep(1);

    /* Send poweroff to init process */
    kill(1, SIGUSR2);
}

void x6100_control_split_set(bool on)
{
    uint32_t prev = x6100_control_get(x6100_sple_atue_trx);
    uint32_t next;

    if (on) {
        next = prev | x6100_sple;
    } else {
        next = prev & (~x6100_sple);
    }

    x6100_control_cmd(x6100_sple_atue_trx, next);
}

void x6100_control_atu_set(bool on)
{
    uint32_t prev = x6100_control_get(x6100_sple_atue_trx);
    uint32_t next;

    if (on) {
        next = prev | x6100_atue;
    } else {
        next = prev & (~x6100_atue);
    }

    x6100_control_cmd(x6100_sple_atue_trx, next);
}

void x6100_control_modem_set(bool on)
{
    uint32_t prev = x6100_control_get(x6100_sple_atue_trx);
    uint32_t next;

    if (on) {
        next = prev | x6100_modem_trx;
    } else {
        next = prev & (~x6100_modem_trx);
    }

    x6100_control_cmd(x6100_sple_atue_trx, next);
}

void x6100_control_swrscan_set(bool on)
{
    uint32_t prev = x6100_control_get(x6100_sple_atue_trx);
    uint32_t next;

    if (on) {
        next = prev | x6100_swrscan_trx;
    } else {
        next = prev & (~x6100_swrscan_trx);
    }

    x6100_control_cmd(x6100_sple_atue_trx, next);
}

void x6100_control_calibration_set(bool on)
{
    uint32_t prev = x6100_control_get(x6100_sple_atue_trx);
    uint32_t next;

    if (on) {
        next = prev | x6100_calibration_trx;
    } else {
        next = prev & (~x6100_calibration_trx);
    }

    x6100_control_cmd(x6100_sple_atue_trx, next);
}

void x6100_control_rfg_set(uint8_t rfg) {
    uint32_t prev = x6100_control_get(x6100_rfg_txpwr) & (~0xFF);

    x6100_control_cmd(x6100_rfg_txpwr, prev | rfg);
}

void x6100_control_txpwr_set(float pwr) {
    uint32_t prev = x6100_control_get(x6100_rfg_txpwr) & (~(0xFF << 8));
    uint8_t  p = pwr * 10.0f;

    x6100_control_cmd(x6100_rfg_txpwr, prev | (p << 8));
}

void x6100_control_output_gain_set(float gain_db) {
    if (gain_db > 25.0f) {
        gain_db = 25.0f;
    } else if (gain_db < -25.0f) {
        gain_db = -25.0f;
    }
    uint32_t prev = x6100_control_get(x6100_rfg_txpwr) & (~(0xFF << 16));
    int8_t p = gain_db * 5.0f;

    x6100_control_cmd(x6100_rfg_txpwr, prev | ((uint8_t)p << 16));
}

void x6100_control_charger_set(bool on) {
    uint32_t prev = x6100_control_get(x6100_micsel_pttmode_chge_spmode_auxiqgen_sqlthr) & (~(1 << 4));

    x6100_control_cmd(x6100_micsel_pttmode_chge_spmode_auxiqgen_sqlthr, prev | (on << 4));
}

void x6100_control_bias_drive_set(uint16_t x) {
    uint32_t prev = x6100_control_get(x6100_biasdrive_biasfinal) & ~(0xFFFF);

    x6100_control_cmd(x6100_biasdrive_biasfinal, prev | x);
}

void x6100_control_bias_final_set(uint16_t x) {
    uint32_t prev = x6100_control_get(x6100_biasdrive_biasfinal) & ~(0xFFFF << 16);

    x6100_control_cmd(x6100_biasdrive_biasfinal, prev | (x << 16));
}

void x6100_control_tx_i_offset_set(int32_t offset) {
    x6100_control_cmd(x6100_txiofs, offset);
}

void x6100_control_tx_q_offset_set(int32_t offset) {
    x6100_control_cmd(x6100_txqofs, offset);
}

void x6100_control_sql_set(uint8_t sql) {
    uint32_t prev = x6100_control_get(x6100_micsel_pttmode_chge_spmode_auxiqgen_sqlthr) & (~(0xFF << 8));

    x6100_control_cmd(x6100_micsel_pttmode_chge_spmode_auxiqgen_sqlthr, prev | (sql << 8));
}

/* Keyer settings */

void x6100_control_key_speed_set(uint8_t wpm) {
    uint32_t prev = x6100_control_get(x6100_ks_km_kimb_cwtone_cwvol_cwtrain) & (~0xFF);

    x6100_control_cmd(x6100_ks_km_kimb_cwtone_cwvol_cwtrain, prev | wpm);
}

void x6100_control_key_mode_set(x6100_key_mode_t mode) {
    uint32_t prev = x6100_control_get(x6100_ks_km_kimb_cwtone_cwvol_cwtrain) & (~(3 << 8));

    x6100_control_cmd(x6100_ks_km_kimb_cwtone_cwvol_cwtrain, prev | ((mode & 3) << 8));
}

void x6100_control_iambic_mode_set(x6100_iambic_mode_t mode) {
    uint32_t prev = x6100_control_get(x6100_ks_km_kimb_cwtone_cwvol_cwtrain) & (~(3 << 10));

    x6100_control_cmd(x6100_ks_km_kimb_cwtone_cwvol_cwtrain, prev | ((mode & 3) << 10));
}

void x6100_control_key_tone_set(uint16_t tone) {
    uint32_t prev = x6100_control_get(x6100_ks_km_kimb_cwtone_cwvol_cwtrain) & (~(0x7FF << 12));

    x6100_control_cmd(x6100_ks_km_kimb_cwtone_cwvol_cwtrain, prev | ((tone & 0x7FF) << 12));
}

void x6100_control_key_vol_set(uint16_t vol) {
    uint32_t prev = x6100_control_get(x6100_ks_km_kimb_cwtone_cwvol_cwtrain) & (~(0x3F << 23));

    x6100_control_cmd(x6100_ks_km_kimb_cwtone_cwvol_cwtrain, prev | ((vol & 0x3F) << 23));
}

void x6100_control_key_train_set(bool train) {
    uint32_t prev = x6100_control_get(x6100_ks_km_kimb_cwtone_cwvol_cwtrain) & (~(1 << 29));

    x6100_control_cmd(x6100_ks_km_kimb_cwtone_cwvol_cwtrain, prev | ((train & 1) << 29));
}

void x6100_control_qsk_time_set(uint16_t time) {
    uint32_t prev = x6100_control_get(x6100_qsktime_kr) & (~(0xFFFF));

    x6100_control_cmd(x6100_qsktime_kr, prev | (time & 0xFFFF));
}

void x6100_control_key_ratio_set(float ratio) {
    uint32_t  prev = x6100_control_get(x6100_qsktime_kr) & (~((uint32_t) 0xFFFF << 16));
    uint16_t  r = ratio * 10.0f;

    x6100_control_cmd(x6100_qsktime_kr, prev | ((r & 0xFFFF) << 16));
}

void x6100_control_linein_set(uint8_t gain) {
    uint32_t prev = x6100_control_get(x6100_ling_loutg_imicg_hmicg) & (~(0xFF));

    x6100_control_cmd(x6100_ling_loutg_imicg_hmicg, prev | gain);
}

void x6100_control_lineout_set(uint8_t gain) {
    uint32_t prev = x6100_control_get(x6100_ling_loutg_imicg_hmicg) & (~(0xFF << 8));

    x6100_control_cmd(x6100_ling_loutg_imicg_hmicg, prev | (gain << 8));
}

void x6100_control_iqout_set(bool on) {
    uint32_t prev = x6100_control_get(x6100_micsel_pttmode_chge_spmode_auxiqgen_sqlthr) & (~(1 << 6));

    x6100_control_cmd(x6100_micsel_pttmode_chge_spmode_auxiqgen_sqlthr, prev | (on << 6));
}

void x6100_control_imic_set(uint8_t gain) {
    uint32_t prev = x6100_control_get(x6100_ling_loutg_imicg_hmicg) & (~(0xFF << 16));

    x6100_control_cmd(x6100_ling_loutg_imicg_hmicg, prev | (gain << 16));
}

void x6100_control_hmic_set(uint8_t gain) {
    uint32_t prev = x6100_control_get(x6100_ling_loutg_imicg_hmicg) & (~((uint32_t)0xFF << 24));

    x6100_control_cmd(x6100_ling_loutg_imicg_hmicg, prev | (gain << 24));
}

void x6100_control_mic_set(x6100_mic_sel_t mic) {
    uint32_t prev = x6100_control_get(x6100_micsel_pttmode_chge_spmode_auxiqgen_sqlthr) & (~3);

    x6100_control_cmd(x6100_micsel_pttmode_chge_spmode_auxiqgen_sqlthr, prev | mic);
}

void x6100_control_vfo_set(x6100_vfo_t vfo) {
    fg_vfo = vfo;

    uint32_t prev = x6100_control_get(x6100_vi_vm) & (~(0xFF));

    x6100_control_cmd(x6100_vi_vm, prev | vfo);
}

/* DSP */

void x6100_control_dnf_set(bool on) {
    uint32_t prev = x6100_control_get(x6100_dnfcnt_dnfwidth_dnfe) & (~(1 << 24));

    x6100_control_cmd(x6100_dnfcnt_dnfwidth_dnfe, prev | (on << 24));
}

void x6100_control_dnf_center_set(uint16_t freq) {
    uint32_t prev = x6100_control_get(x6100_dnfcnt_dnfwidth_dnfe) & (~(0xFFF));

    x6100_control_cmd(x6100_dnfcnt_dnfwidth_dnfe, prev | (freq & 0xFFF));
}

void x6100_control_dnf_width_set(uint16_t hz) {
    uint32_t prev = x6100_control_get(x6100_dnfcnt_dnfwidth_dnfe) & (~(0xFFF << 12));

    x6100_control_cmd(x6100_dnfcnt_dnfwidth_dnfe, prev | ((hz & 0xFFF) << 12));
}

void x6100_control_dnf_update_set(bool on) {
    uint32_t prev = x6100_control_get(x6100_dnfcnt_dnfwidth_dnfe) & (~(1 << 25));

    x6100_control_cmd(x6100_dnfcnt_dnfwidth_dnfe, prev | (on << 25));
}

void x6100_control_nb_set(bool on) {
    uint32_t prev = x6100_control_get(x6100_nrthr_nbw_nbthr_nre_nbe) & (~(1 << 25));

    x6100_control_cmd(x6100_nrthr_nbw_nbthr_nre_nbe, prev | (on << 25));
}

void x6100_control_nb_level_set(uint8_t level) {
    uint32_t prev = x6100_control_get(x6100_nrthr_nbw_nbthr_nre_nbe) & (~(0xFF << 16));

    x6100_control_cmd(x6100_nrthr_nbw_nbthr_nre_nbe, prev | (level << 16));
}

void x6100_control_nb_width_set(uint8_t hz) {
    uint32_t prev = x6100_control_get(x6100_nrthr_nbw_nbthr_nre_nbe) & (~(0xFF << 8));

    x6100_control_cmd(x6100_nrthr_nbw_nbthr_nre_nbe, prev | (hz << 8));
}

void x6100_control_nr_set(bool on) {
    uint32_t prev = x6100_control_get(x6100_nrthr_nbw_nbthr_nre_nbe) & (~(1 << 24));

    x6100_control_cmd(x6100_nrthr_nbw_nbthr_nre_nbe, prev | (on << 24));
}

void x6100_control_nr_level_set(uint8_t level) {
    uint32_t prev = x6100_control_get(x6100_nrthr_nbw_nbthr_nre_nbe) & (~0xFF);

    x6100_control_cmd(x6100_nrthr_nbw_nbthr_nre_nbe, prev | level);
}

/* AGC */

void x6100_control_agc_hang_set(bool on) {
    uint32_t prev = x6100_control_get(x6100_agcknee_agcslope_agchang) & (~(1 << 12));

    x6100_control_cmd(x6100_agcknee_agcslope_agchang, prev | (on << 12));
}

void x6100_control_agc_knee_set(int8_t db) {
    uint32_t prev = x6100_control_get(x6100_agcknee_agcslope_agchang) & (~0xFF);

    x6100_control_cmd(x6100_agcknee_agcslope_agchang, prev | (db & 0xFF));
}

void x6100_control_agc_slope_set(uint8_t db) {
    uint32_t prev = x6100_control_get(x6100_agcknee_agcslope_agchang) & (~(0xFF << 8));

    x6100_control_cmd(x6100_agcknee_agcslope_agchang, prev | (db << 8));
}

void x6100_control_agc_time_set(uint16_t ms) {
    x6100_control_cmd(x6100_agctime, ms);
}

/* VOX */

void x6100_control_vox_set(bool on) {
    uint32_t prev = x6100_control_get(x6100_voxg_voxag_voxdly_voxe) & (~(1 << 26));

    x6100_control_cmd(x6100_voxg_voxag_voxdly_voxe, prev | (on << 26));
}

void x6100_control_vox_ag_set(uint8_t level) {
    uint32_t prev = x6100_control_get(x6100_voxg_voxag_voxdly_voxe) & (~(0x7F << 7));

    x6100_control_cmd(x6100_voxg_voxag_voxdly_voxe, prev | ((level & 0x7F) << 7));
}

void x6100_control_vox_delay_set(uint16_t ms) {
    uint32_t prev = x6100_control_get(x6100_voxg_voxag_voxdly_voxe) & (~(0xFFF << 14));

    x6100_control_cmd(x6100_voxg_voxag_voxdly_voxe, prev | ((ms & 0xFFF) << 14));
}

void x6100_control_vox_gain_set(uint8_t level) {
    uint32_t prev = x6100_control_get(x6100_voxg_voxag_voxdly_voxe) & (~0x7F);

    x6100_control_cmd(x6100_voxg_voxag_voxdly_voxe, prev | (level & 0x7F));
}

/* COMP
0 - 3 - level
4 - on/off
10 - 5 - threshold, 0.5 db, -15.5 ... +16
16 - 11 - makeup, 0.5 db, -15.5 ... +16
*/


void x6100_control_comp_set(bool on)
{
    uint32_t prev = x6100_control_get(x6100_cmplevel_cmpe) & (~(1 << 4));
    x6100_control_cmd(x6100_cmplevel_cmpe, prev | (on << 4));

}

void x6100_control_comp_level_set(x6100_comp_level_t level)
{
    uint32_t prev = x6100_control_get(x6100_cmplevel_cmpe) & (~0xf);
    x6100_control_cmd(x6100_cmplevel_cmpe, prev | (level & 0xf));
}

void x6100_control_comp_threshold_set(float threshold_offset)
{
    // Step = 0.5 db, range -15.5...16
    if (threshold_offset > 16.0f) {
        threshold_offset = 16.0f;
    } else if (threshold_offset < -15.5f) {
        threshold_offset = -15.5f;
    }
    int8_t threshold = threshold_offset * 2;
    uint32_t prev = x6100_control_get(x6100_cmplevel_cmpe) & (~(0x3F << 5));
    x6100_control_cmd(x6100_cmplevel_cmpe, prev | (((uint8_t)threshold & 0x3F) << 5));
}

void x6100_control_comp_makeup_set(float makeup)
{
    // Step = 0.5 db, range -15.5...16
    if (makeup > 16.0f) {
        makeup = 16.0f;
    } else if (makeup < -15.5f) {
        makeup = -15.5f;
    }
    int8_t makeup_int = makeup * 2;
    uint32_t prev = x6100_control_get(x6100_cmplevel_cmpe) & (~(0x3F << 11));
    x6100_control_cmd(x6100_cmplevel_cmpe, prev | (((uint8_t)makeup_int & 0x3F) << 11));
}
