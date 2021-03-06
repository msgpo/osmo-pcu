/* bts.h
 *
 * Copyright (C) 2012 Ivan Klyuchnikov
 * Copyright (C) 2013 by Holger Hans Peter Freyther
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
extern "C" {
#include <osmocom/core/linuxlist.h>
#include <osmocom/core/rate_ctr.h>
#include <osmocom/core/stat_item.h>
#include <osmocom/core/tdef.h>
#include <osmocom/gsm/l1sap.h>
#include <osmocom/gsm/protocol/gsm_04_08.h>
#include "mslot_class.h"
#include "gsm_rlcmac.h"
}

#include "poll_controller.h"
#include "sba.h"
#include "tbf.h"
#include "gprs_ms_storage.h"
#include "coding_scheme.h"
#include <cxx_linuxlist.h>
#endif

#include <pdch.h>
#include <stdint.h>

#define LLC_CODEL_DISABLE 0
#define LLC_CODEL_USE_DEFAULT (-1)

#define MAX_EDGE_MCS 9
#define MAX_GPRS_CS 4

/* see bts->gsmtap_categ_mask */
enum pcu_gsmtap_category {
	PCU_GSMTAP_C_DL_UNKNOWN		= 0,	/* unknown or undecodable downlink blocks */
	PCU_GSMTAP_C_DL_DUMMY		= 1, 	/* downlink dummy blocks */
	PCU_GSMTAP_C_DL_CTRL		= 2,	/* downlink control blocks */
	PCU_GSMTAP_C_DL_DATA_GPRS	= 3,	/* downlink GPRS data blocks */
	PCU_GSMTAP_C_DL_DATA_EGPRS	= 4,	/* downlink EGPRS data blocks */
	PCU_GSMTAP_C_DL_PTCCH		= 5,	/* downlink PTCCH blocks */
	PCU_GSMTAP_C_DL_AGCH		= 6,	/* downlink AGCH blocks */
	PCU_GSMTAP_C_DL_PCH		= 7,	/* downlink PCH blocks */

	PCU_GSMTAP_C_UL_UNKNOWN		= 15,	/* unknown or undecodable uplink blocks */
	PCU_GSMTAP_C_UL_DUMMY		= 16,	/* uplink dummy blocks */
	PCU_GSMTAP_C_UL_CTRL		= 17,	/* uplink control blocks */
	PCU_GSMTAP_C_UL_DATA_GPRS	= 18,	/* uplink GPRS data blocks */
	PCU_GSMTAP_C_UL_DATA_EGPRS	= 19,	/* uplink EGPRS data blocks */
	PCU_GSMTAP_C_UL_RACH		= 20,	/* uplink RACH bursts */
	PCU_GSMTAP_C_UL_PTCCH		= 21,	/* uplink PTCCH bursts */
};

struct BTS;
struct GprsMs;

struct gprs_rlcmac_trx {
	void *fl1h;
	uint16_t arfcn;
	struct gprs_rlcmac_pdch pdch[8];

	/* back pointers */
	struct BTS *bts;
	uint8_t trx_no;

#ifdef __cplusplus
	void reserve_slots(enum gprs_rlcmac_tbf_direction dir, uint8_t slots);
	void unreserve_slots(enum gprs_rlcmac_tbf_direction dir, uint8_t slots);
#endif
};

#ifdef __cplusplus
extern "C" {
#endif
void bts_update_tbf_ta(const char *p, uint32_t fn, uint8_t trx_no, uint8_t ts, int8_t ta, bool is_rach);
#ifdef __cplusplus
}
#endif

/**
 * This is the data from C. As soon as our minimal compiler is gcc 4.7
 * we can start to compile pcu_vty.c with c++ and remove the split.
 */
struct gprs_rlcmac_bts {
	bool active;
	uint8_t bsic;
	uint8_t fc_interval;
	uint16_t fc_bucket_time;
	uint32_t fc_bvc_bucket_size;
	uint32_t fc_bvc_leak_rate;
	uint32_t fc_ms_bucket_size;
	uint32_t fc_ms_leak_rate;
	uint8_t cs1;
	uint8_t cs2;
	uint8_t cs3;
	uint8_t cs4;
	uint8_t initial_cs_dl, initial_cs_ul;
	uint8_t initial_mcs_dl, initial_mcs_ul;
	uint8_t max_cs_dl, max_cs_ul;
	uint8_t max_mcs_dl, max_mcs_ul;
	uint8_t force_cs;	/* 0=use from BTS 1=use from VTY */
	uint16_t force_llc_lifetime; /* overrides lifetime from SGSN */
	uint32_t llc_discard_csec;
	uint32_t llc_idle_ack_csec;
	uint32_t llc_codel_interval_msec; /* 0=disabled, -1=use default interval */
	/* Timer defintions */
	struct osmo_tdef *T_defs_bts; /* timers controlled by BTS, received through PCUIF */
	struct osmo_tdef *T_defs_pcu; /* timers controlled by PCU */
	uint8_t n3101;
	uint8_t n3103;
	uint8_t n3105;
	struct gsmtap_inst *gsmtap;
	uint32_t gsmtap_categ_mask;
	struct gprs_rlcmac_trx trx[8];
	int (*alloc_algorithm)(struct gprs_rlcmac_bts *bts, struct GprsMs *ms, struct gprs_rlcmac_tbf *tbf,
			       bool single, int8_t use_tbf);

	uint8_t force_two_phase;
	uint8_t alpha, gamma;
	uint8_t egprs_enabled;
	bool dl_tbf_preemptive_retransmission;
	uint8_t si13[GSM_MACBLOCK_LEN];
	bool si13_is_set;
	/* 0 to support resegmentation in DL, 1 for no reseg */
	uint8_t dl_arq_type;

	uint8_t cs_adj_enabled; /* whether cs_adj_{upper,lower}_limit are used to adjust DL CS */
	uint8_t cs_adj_upper_limit; /* downgrade DL CS if error rate above its value */
	uint8_t cs_adj_lower_limit; /* upgrade DL CS if error rate below its value */
	/* downgrade DL CS when less than specified octets are left in tx queue. Optimization, see paper:
	  "Theoretical Analysis of GPRS Throughput and Delay" */
	uint16_t cs_downgrade_threshold;
	/* Link quality range for each UL (M)CS. Below or above, next/prev (M)CS is selected. */
	struct {int16_t low; int16_t high; } cs_lqual_ranges[MAX_GPRS_CS];
	struct {int16_t low; int16_t high; } mcs_lqual_ranges[MAX_EDGE_MCS];
	uint16_t ws_base;
	uint16_t ws_pdch; /* increase WS by this value per PDCH */

	/* State for dynamic algorithm selection */
	int multislot_disabled;

	/**
	 * Point back to the C++ object. This is used during the transition
	 * period.
	 */
	struct BTS *bts;

	/* Path to be used for the pcu-bts socket */
	char *pcu_sock_path;

	/* Are we talking Gb with IP-SNS (true) or classic Gb? */
	bool gb_dialect_sns;

	/* Packet Application Information (3GPP TS 44.060 11.2.47, usually ETWS primary message). We don't need to store
	 * more than one message, because they get sent so rarely. */
	struct msgb *app_info;
	uint32_t app_info_pending; /* Count of MS with active TBF, to which we did not send app_info yet */
};

enum {
	CTR_TBF_DL_ALLOCATED,
	CTR_TBF_DL_FREED,
	CTR_TBF_DL_ABORTED,
	CTR_TBF_UL_ALLOCATED,
	CTR_TBF_UL_FREED,
	CTR_TBF_UL_ABORTED,
	CTR_TBF_REUSED,
	CTR_TBF_ALLOC_ALGO_A,
	CTR_TBF_ALLOC_ALGO_B,
	CTR_TBF_FAILED_EGPRS_ONLY,
	CTR_RLC_SENT,
	CTR_RLC_RESENT,
	CTR_RLC_RESTARTED,
	CTR_RLC_STALLED,
	CTR_RLC_NACKED,
	CTR_RLC_FINAL_BLOCK_RESENT,
	CTR_RLC_ASS_TIMEDOUT,
	CTR_RLC_ASS_FAILED,
	CTR_RLC_ACK_TIMEDOUT,
	CTR_RLC_ACK_FAILED,
	CTR_RLC_REL_TIMEDOUT,
	CTR_RLC_LATE_BLOCK,
	CTR_RLC_SENT_DUMMY,
	CTR_RLC_SENT_CONTROL,
	CTR_RLC_DL_BYTES,
	CTR_RLC_DL_PAYLOAD_BYTES,
	CTR_RLC_UL_BYTES,
	CTR_RLC_UL_PAYLOAD_BYTES,
	CTR_DECODE_ERRORS,
	CTR_SBA_ALLOCATED,
	CTR_SBA_FREED,
	CTR_SBA_TIMEDOUT,
	CTR_LLC_FRAME_TIMEDOUT,
	CTR_LLC_FRAME_DROPPED,
	CTR_LLC_FRAME_SCHED,
	CTR_LLC_DL_BYTES,
	CTR_LLC_UL_BYTES,
	CTR_RACH_REQUESTS,
	CTR_11BIT_RACH_REQUESTS,
	CTR_SPB_UL_FIRST_SEGMENT,
	CTR_SPB_UL_SECOND_SEGMENT,
	CTR_SPB_DL_FIRST_SEGMENT,
	CTR_SPB_DL_SECOND_SEGMENT,
	CTR_IMMEDIATE_ASSIGN_UL_TBF,
	CTR_IMMEDIATE_ASSIGN_REJ,
	CTR_IMMEDIATE_ASSIGN_DL_TBF,
	CTR_CHANNEL_REQUEST_DESCRIPTION,
	CTR_PKT_UL_ASSIGNMENT,
	CTR_PKT_ACCESS_REJ,
	CTR_PKT_DL_ASSIGNMENT,
	CTR_RLC_RECV_CONTROL,
	CTR_PUA_POLL_TIMEDOUT,
	CTR_PUA_POLL_FAILED,
	CTR_PDA_POLL_TIMEDOUT,
	CTR_PDA_POLL_FAILED,
	CTR_PUAN_POLL_TIMEDOUT,
	CTR_PUAN_POLL_FAILED,
	CTR_PDAN_POLL_TIMEDOUT,
	CTR_PDAN_POLL_FAILED,
	CTR_GPRS_DL_CS1,
	CTR_GPRS_DL_CS2,
	CTR_GPRS_DL_CS3,
	CTR_GPRS_DL_CS4,
	CTR_EGPRS_DL_MCS1,
	CTR_EGPRS_DL_MCS2,
	CTR_EGPRS_DL_MCS3,
	CTR_EGPRS_DL_MCS4,
	CTR_EGPRS_DL_MCS5,
	CTR_EGPRS_DL_MCS6,
	CTR_EGPRS_DL_MCS7,
	CTR_EGPRS_DL_MCS8,
	CTR_EGPRS_DL_MCS9,
	CTR_GPRS_UL_CS1,
	CTR_GPRS_UL_CS2,
	CTR_GPRS_UL_CS3,
	CTR_GPRS_UL_CS4,
	CTR_EGPRS_UL_MCS1,
	CTR_EGPRS_UL_MCS2,
	CTR_EGPRS_UL_MCS3,
	CTR_EGPRS_UL_MCS4,
	CTR_EGPRS_UL_MCS5,
	CTR_EGPRS_UL_MCS6,
	CTR_EGPRS_UL_MCS7,
	CTR_EGPRS_UL_MCS8,
	CTR_EGPRS_UL_MCS9,
};

enum {
	STAT_MS_PRESENT,
};

/* RACH.ind parameters (to be parsed) */
struct rach_ind_params {
	enum ph_burst_type burst_type;
	bool is_11bit;
	uint16_t ra;
	uint8_t trx_nr;
	uint8_t ts_nr;
	uint32_t rfn;
	int16_t qta;
};

/* [EGPRS Packet] Channel Request parameters (parsed) */
struct chan_req_params {
	unsigned int egprs_mslot_class;
	unsigned int priority;
	bool single_block;
};

#ifdef __cplusplus
/**
 * I represent a GSM BTS. I have one or more TRX, I know the current
 * GSM time and I have controllers that help with allocating resources
 * on my TRXs.
 */
struct BTS {
public:
	BTS();
	~BTS();
	void cleanup();

	static BTS* main_bts();

	struct gprs_rlcmac_bts *bts_data();
	SBAController *sba();

	/** TODO: change the number to unsigned */
	void set_current_frame_number(int frame_number);
	void set_current_block_frame_number(int frame_number, unsigned max_delay);
	int current_frame_number() const;

	/** add paging to paging queue(s) */
	int add_paging(uint8_t chan_needed, const uint8_t *mi, uint8_t mi_len);

	gprs_rlcmac_dl_tbf *dl_tbf_by_poll_fn(uint32_t fn, uint8_t trx, uint8_t ts);
	gprs_rlcmac_ul_tbf *ul_tbf_by_poll_fn(uint32_t fn, uint8_t trx, uint8_t ts);
	gprs_rlcmac_dl_tbf *dl_tbf_by_tfi(uint8_t tfi, uint8_t trx, uint8_t ts);
	gprs_rlcmac_ul_tbf *ul_tbf_by_tfi(uint8_t tfi, uint8_t trx, uint8_t ts);

	int tfi_find_free(enum gprs_rlcmac_tbf_direction dir, uint8_t *_trx, int8_t use_trx) const;

	int rcv_imm_ass_cnf(const uint8_t *data, uint32_t fn);

	uint32_t rfn_to_fn(int32_t rfn);
	int rcv_rach(const struct rach_ind_params *rip);
	int rcv_ptcch_rach(const struct rach_ind_params *rip);

	void snd_dl_ass(gprs_rlcmac_tbf *tbf, bool poll, uint16_t pgroup);

	GprsMsStorage &ms_store();
	GprsMs *ms_by_tlli(uint32_t tlli, uint32_t old_tlli = 0);
	GprsMs *ms_by_imsi(const char *imsi);
	GprsMs *ms_alloc(uint8_t ms_class, uint8_t egprs_ms_class = 0);

	void send_gsmtap(enum pcu_gsmtap_category categ, bool uplink, uint8_t trx_no,
			      uint8_t ts_no, uint8_t channel, uint32_t fn,
			      const uint8_t *data, unsigned int len);
	void send_gsmtap_meas(enum pcu_gsmtap_category categ, bool uplink, uint8_t trx_no,
			      uint8_t ts_no, uint8_t channel, uint32_t fn,
			      const uint8_t *data, unsigned int len, struct pcu_l1_meas *meas);
	void send_gsmtap_rach(enum pcu_gsmtap_category categ, uint8_t channel,
			      const struct rach_ind_params *rip);

	/*
	 * Below for C interface for the VTY
	 */
	struct rate_ctr_group *rate_counters() const;
	struct osmo_stat_item_group *stat_items() const;
	void do_rate_ctr_inc(unsigned int ctr_id);
	void do_rate_ctr_add(unsigned int ctr_id, int inc);
	void stat_item_add(unsigned int stat_id, int inc);

	LListHead<gprs_rlcmac_tbf>& ul_tbfs();
	LListHead<gprs_rlcmac_tbf>& dl_tbfs();
private:
	int m_cur_fn;
	int m_cur_blk_fn;
	struct gprs_rlcmac_bts m_bts;
	PollController m_pollController;
	SBAController m_sba;
	struct rate_ctr_group *m_ratectrs;
	struct osmo_stat_item_group *m_statg;

	GprsMsStorage m_ms_store;

	/* list of uplink TBFs */
	LListHead<gprs_rlcmac_tbf> m_ul_tbfs;
	/* list of downlink TBFs */
	LListHead<gprs_rlcmac_tbf> m_dl_tbfs;

	/* disable copying to avoid slicing */
	BTS(const BTS&);
	BTS& operator=(const BTS&);
};

inline int BTS::current_frame_number() const
{
	return m_cur_fn;
}

inline SBAController *BTS::sba()
{
	return &m_sba;
}

inline GprsMsStorage &BTS::ms_store()
{
	return m_ms_store;
}

inline GprsMs *BTS::ms_by_tlli(uint32_t tlli, uint32_t old_tlli)
{
	return ms_store().get_ms(tlli, old_tlli);
}

inline GprsMs *BTS::ms_by_imsi(const char *imsi)
{
	return ms_store().get_ms(0, 0, imsi);
}

inline LListHead<gprs_rlcmac_tbf>& BTS::ul_tbfs()
{
	return m_ul_tbfs;
}

inline LListHead<gprs_rlcmac_tbf>& BTS::dl_tbfs()
{
	return m_dl_tbfs;
}

inline struct rate_ctr_group *BTS::rate_counters() const
{
	return m_ratectrs;
}

inline struct osmo_stat_item_group *BTS::stat_items() const
{
	return m_statg;
}

inline void BTS::do_rate_ctr_inc(unsigned int ctr_id) {
	rate_ctr_inc(&m_ratectrs->ctr[ctr_id]);
}

inline void BTS::do_rate_ctr_add(unsigned int ctr_id, int inc) {
	rate_ctr_add(&m_ratectrs->ctr[ctr_id], inc);
}

inline void BTS::stat_item_add(unsigned int stat_id, int inc) {
	int32_t val = osmo_stat_item_get_last(m_statg->items[stat_id]);
	osmo_stat_item_set(m_statg->items[stat_id], val + inc);
}

#endif

#ifdef __cplusplus
extern "C" {
#endif
	void bts_cleanup();
	struct gprs_rlcmac_bts *bts_main_data();
	struct rate_ctr_group *bts_main_data_stats();
	struct osmo_stat_item_group *bts_main_data_stat_items();
#ifdef __cplusplus
}

#endif
