/*
 * Copyright (C) 2013 by Holger Hans Peter Freyther
 * Copyright (C) 2019 by sysmocom - s.f.m.c. GmbH <info@sysmocom.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#pragma once

#ifdef __cplusplus

#include <stdbool.h>

#include "tbf.h"
/*
 * TBF instance
 */

enum tbf_gprs_ul_counters {
        TBF_CTR_GPRS_UL_CS1,
        TBF_CTR_GPRS_UL_CS2,
        TBF_CTR_GPRS_UL_CS3,
        TBF_CTR_GPRS_UL_CS4,
};

enum tbf_egprs_ul_counters {
        TBF_CTR_EGPRS_UL_MCS1,
        TBF_CTR_EGPRS_UL_MCS2,
        TBF_CTR_EGPRS_UL_MCS3,
        TBF_CTR_EGPRS_UL_MCS4,
        TBF_CTR_EGPRS_UL_MCS5,
        TBF_CTR_EGPRS_UL_MCS6,
        TBF_CTR_EGPRS_UL_MCS7,
        TBF_CTR_EGPRS_UL_MCS8,
        TBF_CTR_EGPRS_UL_MCS9,
};

#define LOGPTBFUL(tbf, level, fmt, args...) LOGP(DTBFUL, level, "%s " fmt, tbf_name(tbf), ## args)

struct gprs_rlcmac_ul_tbf : public gprs_rlcmac_tbf {
	gprs_rlcmac_ul_tbf(BTS *bts);
	gprs_rlc_ul_window *window();
	struct msgb *create_ul_ack(uint32_t fn, uint8_t ts);
	bool ctrl_ack_to_toggle();
	bool handle_ctrl_ack();
	void enable_egprs();
	/* blocks were acked */
	int rcv_data_block_acknowledged(
		const struct gprs_rlc_data_info *rlc,
		uint8_t *data, struct pcu_l1_meas *meas);


	/* TODO: extract LLC class? */
	int assemble_forward_llc(const gprs_rlc_data *data);
	int snd_ul_ud();

	egprs_rlc_ul_reseg_bsn_state handle_egprs_ul_spb(
		const struct gprs_rlc_data_info *rlc,
		struct gprs_rlc_data *block,
		uint8_t *data, const uint8_t block_idx);

	egprs_rlc_ul_reseg_bsn_state handle_egprs_ul_first_seg(
		const struct gprs_rlc_data_info *rlc,
		struct gprs_rlc_data *block,
		uint8_t *data, const uint8_t block_idx);

	egprs_rlc_ul_reseg_bsn_state handle_egprs_ul_second_seg(
		const struct gprs_rlc_data_info *rlc,
		struct gprs_rlc_data *block,
		uint8_t *data, const uint8_t block_idx);

	uint16_t window_size() const;
	void set_window_size();
	void update_coding_scheme_counter_ul(enum CodingScheme cs);

	/* Please note that all variables here will be reset when changing
	 * from WAIT RELEASE back to FLOW state (re-use of TBF).
	 * All states that need reset must be in this struct, so this is why
	 * variables are in both (dl and ul) structs and not outside union.
	 */
	int32_t m_rx_counter; /* count all received blocks */
	uint8_t m_usf[8];	/* list USFs per PDCH (timeslot) */
	uint8_t m_contention_resolution_done; /* set after done */
	uint8_t m_final_ack_sent; /* set if we sent final ack */

	struct rate_ctr_group *m_ul_gprs_ctrs;
	struct rate_ctr_group *m_ul_egprs_ctrs;

protected:
	void maybe_schedule_uplink_acknack(const gprs_rlc_data_info *rlc, bool countdown_finished);

	/* Please note that all variables below will be reset when changing
	 * from WAIT RELEASE back to FLOW state (re-use of TBF).
	 * All states that need reset must be in this struct, so this is why
	 * variables are in both (dl and ul) structs and not outside union.
	 */
	gprs_rlc_ul_window m_window;
};

#ifdef __cplusplus
extern "C" {
#endif
void update_tbf_ta(struct gprs_rlcmac_ul_tbf *tbf, int8_t ta_delta);
void set_tbf_ta(struct gprs_rlcmac_ul_tbf *tbf, uint8_t ta);
#ifdef __cplusplus
}
#endif

inline uint16_t gprs_rlcmac_ul_tbf::window_size() const
{
	return m_window.ws();
}

inline void gprs_rlcmac_ul_tbf::enable_egprs()
{
	m_window.set_sns(RLC_EGPRS_SNS);
	gprs_rlcmac_tbf::enable_egprs();
}

inline gprs_rlcmac_ul_tbf *as_ul_tbf(gprs_rlcmac_tbf *tbf)
{
	if (tbf && tbf->direction == GPRS_RLCMAC_UL_TBF)
		return static_cast<gprs_rlcmac_ul_tbf *>(tbf);
	else
		return NULL;
}

#endif
