
#include "wifi_hal.h"
#include "gscan.h"

#ifndef __WIFI_HAL_RTT_H__
#define __WIFI_HAL_RTT_H__

/* Ranging status */
typedef enum {
    RTT_STATUS_SUCCESS       = 0,
    RTT_STATUS_FAILURE       = 1,           // general failure status
    RTT_STATUS_FAIL_NO_RSP   = 2,           // target STA does not respond to request
    RTT_STATUS_FAIL_REJECTED = 3,           // request rejected. Applies to 2-sided RTT only
    RTT_STATUS_FAIL_NOT_SCHEDULED_YET  = 4,
    RTT_STATUS_FAIL_TM_TIMEOUT         = 5, // timing measurement times out
    RTT_STATUS_FAIL_AP_ON_DIFF_CHANNEL = 6, // Target on different channel, cannot range
    RTT_STATUS_FAIL_NO_CAPABILITY  = 7,     // ranging not supported
    RTT_STATUS_ABORTED             = 8,     // request aborted for unknown reason
    RTT_STATUS_FAIL_INVALID_TS     = 9,     // Invalid T1-T4 timestamp
    RTT_STATUS_FAIL_PROTOCOL       = 10,    // 11mc protocol failed
    RTT_STATUS_FAIL_SCHEDULE       = 11,    // request could not be scheduled
    RTT_STATUS_FAIL_BUSY_TRY_LATER = 12,    // responder cannot collaborate at time of request
    RTT_STATUS_INVALID_REQ         = 13,    // bad request args
    RTT_STATUS_NO_WIFI             = 14,    // WiFi not enabled
    RTT_STATUS_FAIL_FTM_PARAM_OVERRIDE = 15 // Responder overrides param info, cannot range with new params
} wifi_rtt_status;

/* RTT peer type */
typedef enum {
    RTT_PEER_AP         = 0x1,
    RTT_PEER_STA        = 0x2,
    RTT_PEER_P2P_GO     = 0x3,
    RTT_PEER_P2P_CLIENT = 0x4,
    RTT_PEER_NAN        = 0x5
} rtt_peer_type;

/* RTT Measurement Bandwidth */
typedef enum {
    WIFI_RTT_BW_5   = 0x01,
    WIFI_RTT_BW_10  = 0x02,
    WIFI_RTT_BW_20  = 0x04,
    WIFI_RTT_BW_40  = 0x08,
    WIFI_RTT_BW_80  = 0x10,
    WIFI_RTT_BW_160 = 0x20
} wifi_rtt_bw;

/* RTT Measurement Preamble */
typedef enum {
    WIFI_RTT_PREAMBLE_LEGACY = 0x1,
    WIFI_RTT_PREAMBLE_HT     = 0x2,
    WIFI_RTT_PREAMBLE_VHT    = 0x4
} wifi_rtt_preamble;

/* RTT Type */
typedef enum {
    RTT_TYPE_1_SIDED = 0x1,
    RTT_TYPE_2_SIDED = 0x2,
} wifi_rtt_type;

/* RTT configuration */
typedef struct {
    mac_addr addr;                 // peer device mac address
    wifi_rtt_type type;            // 1-sided or 2-sided RTT
    rtt_peer_type peer;            // optional - peer device hint (STA, P2P, AP)
    wifi_channel_info channel;     // Required for STA-AP mode, optional for P2P, NBD etc.
    unsigned burst_period;         // Time interval between bursts (units: 100 ms).
                                   // Applies to 1-sided and 2-sided RTT multi-burst requests.
                                   // Range: 0-31, 0: no preference by initiator (2-sided RTT)
    unsigned num_burst;            // Total number of RTT bursts to be executed. It will be
                                   // specified in the same way as the parameter "Number of
                                   // Burst Exponent" found in the FTM frame format. It
                                   // applies to both: 1-sided RTT and 2-sided RTT. Valid
                                   // values are 0 to 15 as defined in 802.11mc std.
                                   // 0 means single shot
                                   // The implication of this parameter on the maximum
                                   // number of RTT results is the following:
                                   // for 1-sided RTT: max num of RTT results = (2^num_burst)*(num_frames_per_burst)
                                   // for 2-sided RTT: max num of RTT results = (2^num_burst)*(num_frames_per_burst - 1)
    unsigned num_frames_per_burst; // num of frames per burst.
                                   // Minimum value = 1, Maximum value = 31
                                   // For 2-sided this equals the number of FTM frames
                                   // to be attempted in a single burst. This also
                                   // equals the number of FTM frames that the
                                   // initiator will request that the responder send
                                   // in a single frame.
    unsigned num_retries_per_rtt_frame; // number of retries for a failed RTT frame. Applies
                                        // to 1-sided RTT only. Minimum value = 0, Maximum value = 3

    //following fields are only valid for 2-side RTT
    unsigned num_retries_per_ftmr; // Maximum number of retries that the initiator can
                                   // retry an FTMR frame.
                                   // Minimum value = 0, Maximum value = 3
    byte LCI_request;              // 1: request LCI, 0: do not request LCI
    byte LCR_request;              // 1: request LCR, 0: do not request LCR
    unsigned burst_duration;       // Applies to 1-sided and 2-sided RTT. Valid values will
                                   // be 2-11 and 15 as specified by the 802.11mc std for
                                   // the FTM parameter burst duration. In a multi-burst
                                   // request, if responder overrides with larger value,
                                   // the initiator will return failure. In a single-burst
                                   // request if responder overrides with larger value,
                                   // the initiator will sent TMR_STOP to terminate RTT
                                   // at the end of the burst_duration it requested.
    wifi_rtt_preamble preamble;    // RTT preamble to be used in the RTT frames
    wifi_rtt_bw bw;                // RTT BW to be used in the RTT frames
} wifi_rtt_config;

/* RTT results */
typedef struct {
    mac_addr addr;                // device mac address
    unsigned burst_num;           // burst number in a multi-burst request
    unsigned measurement_number;  // Total RTT measurement frames attempted
    unsigned success_number;      // Total successful RTT measurement frames
    byte  number_per_burst_peer;  // Maximum number of "FTM frames per burst" supported by
                                  // the responder STA. Applies to 2-sided RTT only.
                                  // If reponder overrides with larger value:
                                  // - for single-burst request initiator will truncate the
                                  // larger value and send a TMR_STOP after receiving as
                                  // many frames as originally requested.
                                  // - for multi-burst request, initiator will return
                                  // failure right away.
    wifi_rtt_status status;       // ranging status
    byte retry_after_duration;    // When status == RTT_STATUS_FAIL_BUSY_TRY_LATER,
                                  // this will be the time provided by the responder as to
                                  // when the request can be tried again. Applies to 2-sided
                                  // RTT only. In sec, 1-31sec.
    wifi_rtt_type type;           // RTT type
    wifi_rssi rssi;               // average rssi in 0.5 dB steps e.g. 143 implies -71.5 dB
    wifi_rssi rssi_spread;        // rssi spread in 0.5 dB steps e.g. 5 implies 2.5 dB spread (optional)
    wifi_rate tx_rate;            // 1-sided RTT: TX rate of RTT frame.
                                  // 2-sided RTT: TX rate of initiator's Ack in response to FTM frame.
    wifi_rate rx_rate;            // 1-sided RTT: TX rate of Ack from other side.
                                  // 2-sided RTT: TX rate of FTM frame coming from responder.
    wifi_timespan rtt;            // round trip time in picoseconds
    wifi_timespan rtt_sd;         // rtt standard deviation in picoseconds
    wifi_timespan rtt_spread;     // difference between max and min rtt times recorded in picoseconds
    int distance_mm;              // distance in mm (optional)
    int distance_sd_mm;           // standard deviation in mm (optional)
    int distance_spread_mm;       // difference between max and min distance recorded in mm (optional)
    wifi_timestamp ts;            // time of the measurement (in microseconds since boot)
    int burst_duration;           // in ms, actual time taken by the FW to finish one burst
                                  // measurement. Applies to 1-sided and 2-sided RTT.
    int negotiated_burst_num;     // Number of bursts allowed by the responder. Applies
                                  // to 2-sided RTT only.
    wifi_information_element *LCI; // for 11mc only
    wifi_information_element *LCR; // for 11mc only
} wifi_rtt_result;

/* RTT result callback */
typedef struct {
    void (*on_rtt_results) (wifi_request_id id, unsigned num_results, wifi_rtt_result *rtt_result[]);
} wifi_rtt_event_handler;

/* API to request RTT measurement */
wifi_error wifi_rtt_range_request(wifi_request_id id, wifi_interface_handle iface,
        unsigned num_rtt_config, wifi_rtt_config rtt_config[], wifi_rtt_event_handler handler);

/* API to cancel RTT measurements */
wifi_error wifi_rtt_range_cancel(wifi_request_id id,  wifi_interface_handle iface,
        unsigned num_devices, mac_addr addr[]);

/* NBD ranging channel map */
typedef struct {
    wifi_channel availablity[32]; // specifies the channel map for each of the 16 TU windows
    // frequency of 0 => unspecified; which means firmware is
    // free to do whatever it wants in this window.
} wifi_channel_map;

/* API to start publishing the channel map on responder device in a NBD cluster.
   Responder device will take this request and schedule broadcasting the channel map
   in a NBD ranging attribute in a SDF. DE will automatically remove the ranging
   attribute from the OTA queue after number of DW specified by num_dw
   where Each DW is 512 TUs apart */
wifi_error wifi_rtt_channel_map_set(wifi_request_id id,
        wifi_interface_handle iface, wifi_channel_map *params, unsigned num_dw);

/* API to clear the channel map on the responder device in a NBD cluster.
   Responder device will cancel future ranging channel request, starting from next
   DW interval and will also stop broadcasting NBD ranging attribute in SDF */
wifi_error wifi_rtt_channel_map_clear(wifi_request_id id,  wifi_interface_handle iface);

// Preamble definition for bit mask used in wifi_rtt_capabilities
#define PREAMBLE_LEGACY 0x1
#define PREAMBLE_HT     0x2
#define PREAMBLE_VHT    0x4

// BW definition for bit mask used in wifi_rtt_capabilities
#define BW_5_SUPPORT   0x1
#define BW_10_SUPPORT  0x2
#define BW_20_SUPPORT  0x4
#define BW_40_SUPPORT  0x8
#define BW_80_SUPPORT  0x10
#define BW_160_SUPPORT 0x20

/* RTT Capabilities */
typedef struct {
    byte rtt_one_sided_supported;  // if 1-sided rtt data collection is supported
    byte rtt_ftm_supported;        // if ftm rtt data collection is supported
    byte lci_support;              // if initiator supports LCI request. Applies to 2-sided RTT
    byte lcr_support;              // if initiator supports LCR request. Applies to 2-sided RTT
    byte preamble_support;         // bit mask indicates what preamble is supported by initiator
    byte bw_support;               // bit mask indicates what BW is supported by initiator
    byte responder_supported;      // if 11mc responder mode is supported
    byte mc_version;               // draft 11mc spec version supported by chip. For instance,
                                   // version 4.0 should be 40 and version 4.3 should be 43 etc.
} wifi_rtt_capabilities;

/*  RTT capabilities of the device */
wifi_error wifi_get_rtt_capabilities(wifi_interface_handle iface, wifi_rtt_capabilities *capabilities);

/* debugging definitions */
enum {
    RTT_DEBUG_DISABLE,
    RTT_DEBUG_LOG,
    RTT_DEBUG_PROTO,
    RTT_DEBUG_BURST,
    RTT_DEBUG_ACCURACY,
    RTT_DEBUG_LOGDETAIL
};  //rtt debug type

enum {
    RTT_DEBUG_FORMAT_TXT,
    RTT_DEBUG_FORMAT_BINARY
}; //rtt debug format

typedef struct rtt_debug {
    unsigned version;
    unsigned len; // total length of after len field
    unsigned type;  // rtt debug type
    unsigned format; //rtt debug format
    char dbuf[0]; // debug content
} rtt_debug_t;

/* set configuration for debug */
wifi_error wifi_rtt_debug_cfg(wifi_interface_handle h, unsigned rtt_dbg_type, char *cfgbuf, unsigned cfg_buf_size);
/* get the debug information */
wifi_error wifi_rtt_debug_get(wifi_interface_handle h, rtt_debug_t **debugbuf);
/* free the debug buffer */
wifi_error wifi_rtt_debug_free(wifi_interface_handle h, rtt_debug_t *debugbuf);

/* API for setting LCI/LCR information to be provided to a requestor */
typedef enum {
    WIFI_MOTION_NOT_EXPECTED = 0, // Not expected to change location
    WIFI_MOTION_EXPECTED = 1,     // Expected to change location
    WIFI_MOTION_UNKNOWN  = 2,     // Movement pattern unknown
} wifi_motion_pattern;

typedef struct {
    long latitude;              // latitude in degrees * 2^25 , 2's complement
    long longitude;             // latitude in degrees * 2^25 , 2's complement
    int  altitude;              // Altitude in units of 1/256 m
    byte latitude_unc;          // As defined in Section 2.3.2 of IETF RFC 6225
    byte longitude_unc;         // As defined in Section 2.3.2 of IETF RFC 6225
    byte altitude_unc;          // As defined in Section 2.4.5 from IETF RFC 6225:

    //Following element for configuring the Z subelement
    wifi_motion_pattern motion_pattern;
    int  floor;                 // floor in units of 1/16th of floor. 0x80000000 if unknown.
    int  height_above_floor;    // in units of 1/64 m
    int  height_unc;            // in units of 1/64 m. 0 if unknown
} wifi_lci_information;

typedef struct {
    char country_code[2];       // country code
    int  length;                // length of the info field
    char civic_info[256];       // Civic info to be copied in FTM frame
} wifi_lcr_information;

// API to configure the LCI. Used in RTT Responder mode only
wifi_error wifi_set_lci(wifi_request_id id, wifi_interface_handle iface,
                        wifi_lci_information *lci);

// API to configure the LCR. Used in RTT Responder mode only.
wifi_error wifi_set_lcr(wifi_request_id id, wifi_interface_handle iface,
                        wifi_lcr_information *lcr);

/**
 * RTT Responder information
 */
typedef struct {
    wifi_channel_info channel;
    wifi_rtt_preamble preamble;
} wifi_rtt_responder;

/**
 * Get RTT responder information e.g. WiFi channel to enable responder on.
 */
wifi_error wifi_rtt_get_responder_info(wifi_interface_handle iface,
                                       wifi_rtt_responder *responder_info);

/**
 * Enable RTT responder mode.
 * channel_hint - hint of the channel information where RTT responder should be enabled on.
 * max_duration_seconds - timeout of responder mode.
 * channel_used - channel used for RTT responder, NULL if responder is not enabled.
 */
wifi_error wifi_enable_responder(wifi_request_id id, wifi_interface_handle iface,
                                 wifi_channel_info channel_hint, unsigned max_duration_seconds,
                                 wifi_rtt_responder *responder_info);

/**
 * Disable RTT responder mode.
 */
wifi_error wifi_disable_responder(wifi_request_id id, wifi_interface_handle iface);

#endif
