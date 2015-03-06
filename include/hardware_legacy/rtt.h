
#include "wifi_hal.h"
#include "gscan.h"

#ifndef __WIFI_HAL_RTT_H__
#define __WIFI_HAL_RTT_H__

/* channel operating width */

/* Ranging status */
typedef enum {
    RTT_STATUS_SUCCESS,
    RTT_STATUS_FAILURE,
    RTT_STATUS_FAIL_NO_RSP,
    RTT_STATUS_FAIL_INVALID_TS, // Invalid T1-T4 timestamp
    RTT_STATUS_FAIL_PROTOCOL,   // 11mc protocol failed
    RTT_STATUS_FAIL_REJECTED,
    RTT_STATUS_FAIL_NOT_SCHEDULED_YET,
    RTT_STATUS_FAIL_SCHEDULE,  // schedule failed
    RTT_STATUS_FAIL_TM_TIMEOUT,
    RTT_STATUS_FAIL_AP_ON_DIFF_CHANNEL,
    RTT_STATUS_FAIL_NO_CAPABILITY,
    RTT_STATUS_FAIL_BUSY_TRY_LATER,        //2 side RTT if other side temporarily busy
    RTT_STATUS_ABORTED
} wifi_rtt_status;


/* RTT Type */
typedef enum {
    RTT_TYPE_INVALID,
    RTT_TYPE_1_SIDED,
    RTT_TYPE_2_SIDED,
    RTT_TYPE_AUTO,              // Two sided if remote supports; one sided otherwise
} wifi_rtt_type;

/* RTT configuration */
typedef struct {
    mac_addr addr;                     // peer device mac address
    wifi_rtt_type type;                // optional - rtt type hint.
                                       // RTT_TYPE_AUTO implies best effort
    wifi_peer_type peer;               // optional - peer device hint (STA, P2P, AP)
    wifi_channel_info channel;         // Required for STA-AP mode, optional for P2P, NBD etc.
    unsigned interval;                 // interval between RTT burst (unit ms).
                                       // Only valid when multi_burst = 1
    unsigned num_burst;                // total number of RTT bursts, 1 means single shot
    unsigned num_frames_per_burst;     // num of frames in each RTT burst
                                       // for single side, measurement result num = frame number
                                       // for 2 side RTT, measurement result num  = frame number - 1
    unsigned num_retries_per_measurement_frame; // retry time for RTT MEASUREMENT frame

    //following fields are only valid for 2 side RTT
    unsigned num_retries_per_ftmr;
    byte LCI_request;                  // request LCI or not
    byte LCR_request;                  // request LCR or not
    unsigned burst_timeout;            // unit of 250 us
    byte preamble;                     // 0- Legacy,1- HT, 2-VHT
    byte bw;                           //5, 10, 20, 40, 80,160
} wifi_rtt_config;

/* RTT results*/
typedef struct {
    mac_addr addr;               // device mac address
    unsigned burst_num;          // # of burst inside a multi-burst request
    unsigned measurement_number;  // total RTT measurement Frames
    unsigned success_number;     // total successful RTT measurement Frames
    byte  number_per_burst_peer;  //Max number of FTM numbers per burst the other side support,
                                  //11mc only
    wifi_rtt_status status;      // ranging status
    byte retry_after_duration;      // in s , 11mc only, only for RTT_STATUS_FAIL_BUSY_TRY_LATER, 1-31s
    wifi_rtt_type type;          // RTT type
    wifi_rssi rssi;              // average rssi in 0.5 dB steps e.g. 143 implies -71.5 dB
    wifi_rssi rssi_spread;       // rssi spread in 0.5 dB steps e.g. 5 implies 2.5 dB spread (optional)
    wifi_rate tx_rate;           // TX rate
    wifi_rate rx_rate;          // Rx rate
    wifi_timespan rtt;           // round trip time in 0.1 nanoseconds
    wifi_timespan rtt_sd;        // rtt standard deviation in 0.1 nanoseconds
    wifi_timespan rtt_spread;    // difference between max and min rtt times recorded
    int distance;                // distance in cm (optional)
    int distance_sd;             // standard deviation in cm (optional)
    int distance_spread;         // difference between max and min distance recorded (optional)
    wifi_timestamp ts;           // time of the measurement (in microseconds since boot)
    int burst_duration;          // in ms, How long the FW time is to finish one burst measurement
    wifi_information_element LCI;      // for 11mc only, optional
    wifi_information_element LCR;      // for 11mc only, optional
} wifi_rtt_result;

/* RTT result callback */
typedef struct {
    void (*on_rtt_results) (wifi_request_id id, unsigned num_results, wifi_rtt_result rtt_result[]);
} wifi_rtt_event_handler;

/* API to request RTT measurement */
wifi_error wifi_rtt_range_request(wifi_request_id id, wifi_interface_handle iface,
        unsigned num_rtt_config, wifi_rtt_config rtt_config[], wifi_rtt_event_handler handler);

/* API to cancel RTT measurements */
wifi_error wifi_rtt_range_cancel(wifi_request_id id,  wifi_interface_handle iface,
        unsigned num_devices, mac_addr addr[]);

/* NBD ranging channel map */
typedef struct {
    wifi_channel availablity[32];           // specifies the channel map for each of the 16 TU windows
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

#define PREAMBLE_LEGACY 0x1
#define PREAMBLE_HT     0x2
#define PREAMBLE_VHT    0x4

#define BW_5_SUPPORT     0x1
#define BW_10_SUPPORT    0x2
#define BW_20_SUPPORT    0x4
#define BW_40_SUPPORT    0x8
#define BW_80_SUPPORT    0x10
#define BW_160_SUPPORT   0x20

/* RTT Capabilities */
typedef struct {
   byte rtt_one_sided_supported;  // if 1-sided rtt data collection is supported
   byte rtt_ftm_supported;        // if ftm rtt data collection is supported
   byte lci_support;
   byte lcr_support;
   byte preamble_support;         //bit mask indicate what preamble is supported
   byte bw_support;               //bit mask indicate what BW is supported
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
wifi_error wifi_rtt_debug_cfg(wifi_interface_handle h, unsigned rtt_dbg_type, char *cfgbuf, u32 cfg_buf_size);
/* get the debug information */
wifi_error wifi_rtt_debug_get(wifi_interface_handle h, rtt_debug_t **debugbuf);
/* free the debug buffer */
wifi_error wifi_rtt_debug_free(wifi_interface_handle h, rtt_debug_t *debugbuf);

#endif

