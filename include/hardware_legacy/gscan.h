#include "wifi_hal.h"

#ifndef __WIFI_HAL_GSCAN_H__
#define __WIFI_HAL_GSCAN_H__

/* AP Scans */

typedef enum {
    WIFI_BAND_UNSPECIFIED,
    WIFI_BAND_BG = 1,                       // 2.4 GHz
    WIFI_BAND_A = 2,                        // 5 GHz without DFS
    WIFI_BAND_A_DFS = 4,                    // 5 GHz DFS only
    WIFI_BAND_A_WITH_DFS = 6,               // 5 GHz with DFS
    WIFI_BAND_ABG = 3,                      // 2.4 GHz + 5 GHz; no DFS
    WIFI_BAND_ABG_WITH_DFS = 7,             // 2.4 GHz + 5 GHz with DFS
} wifi_band;

const unsigned MAX_CHANNELS                = 16;
const unsigned MAX_BUCKETS                 = 16;
const unsigned MAX_HOTLIST_APS             = 128;
const unsigned MAX_SIGNIFICANT_CHANGE_APS  = 64;
const unsigned MAX_EPNO_NETWORKS           = 64;
const unsigned MAX_HOTLIST_SSID            = 8;
const unsigned MAX_BLACKLIST_BSSID         = 16;
const unsigned MAX_AP_CACHE_PER_SCAN       = 32;

wifi_error wifi_get_valid_channels(wifi_interface_handle handle,
        int band, int max_channels, wifi_channel *channels, int *num_channels);

typedef struct {
    int max_scan_cache_size;                 // total space allocated for scan (in bytes)
    int max_scan_buckets;                    // maximum number of channel buckets
    int max_ap_cache_per_scan;               // maximum number of APs that can be stored per scan
    int max_rssi_sample_size;                // number of RSSI samples used for averaging RSSI
    int max_scan_reporting_threshold;        // max possible report_threshold as described
                                             // in wifi_scan_cmd_params
    int max_hotlist_bssids;                  // maximum number of entries for hotlist BSSIDs
    int max_hotlist_ssids;                   // maximum number of entries for hotlist SSIDs
    int max_significant_wifi_change_aps;     // maximum number of entries for
                                             // significant wifi change APs
    int max_bssid_history_entries;           // number of BSSID/RSSI entries that device can hold
    int max_number_epno_networks;            // max number of epno entries
    int max_number_epno_networks_by_ssid;    // max number of epno entries if ssid is specified,
                                             // that is, epno entries for which an exact match is
                                             // required, or entries corresponding to hidden ssids
    int max_number_of_white_listed_ssid;     // max number of white listed SSIDs, M target is 2 to 4
} wifi_gscan_capabilities;

wifi_error wifi_get_gscan_capabilities(wifi_interface_handle handle,
        wifi_gscan_capabilities *capabilities);

typedef enum {
    WIFI_SCAN_RESULTS_AVAILABLE,   // reported when REPORT_EVENTS_EACH_SCAN is set and a scan
                                   // completes. WIFI_SCAN_THRESHOLD_NUM_SCANS or
                                   // WIFI_SCAN_THRESHOLD_PERCENT can be reported instead if the
                                   // reason for the event is available; however, at most one of
                                   // these events should be reported per scan. If there are
                                   // multiple buckets that were scanned this period and one has the
                                   // EACH_SCAN flag set then this event should be prefered.
    WIFI_SCAN_THRESHOLD_NUM_SCANS, // can be reported when REPORT_EVENTS_EACH_SCAN is not set and
                                   // report_threshold_num_scans is reached.
    WIFI_SCAN_THRESHOLD_PERCENT,   // can be reported when REPORT_EVENTS_EACH_SCAN is not set and
                                   // report_threshold_percent is reached.
    WIFI_SCAN_FAILED,              // reported when currently executing gscans have failed.
                                   // start_gscan will need to be called again in order to continue
                                   // scanning. This is intended to indicate abnormal scan
                                   // terminations (not those as a result of stop_gscan).
} wifi_scan_event;


/* Format of information elements found in the beacon */
typedef struct {
    byte id;                            // element identifier
    byte len;                           // number of bytes to follow
    byte data[];
} wifi_information_element;

typedef struct {
    wifi_timestamp ts;                  // time since boot (in microsecond) when the result was
                                        // retrieved
    char ssid[32+1];                    // null terminated
    mac_addr bssid;
    wifi_channel channel;               // channel frequency in MHz
    wifi_rssi rssi;                     // in db
    wifi_timespan rtt;                  // in nanoseconds
    wifi_timespan rtt_sd;               // standard deviation in rtt
    unsigned short beacon_period;       // period advertised in the beacon
    unsigned short capability;          // capabilities advertised in the beacon
    unsigned int ie_length;             // size of the ie_data blob
    char         ie_data[1];            // blob of all the information elements found in the
                                        // beacon; this data should be a packed list of
                                        // wifi_information_element objects, one after the other.
    // other fields
} wifi_scan_result;

static_assert(MAX_BUCKETS <= 8 * sizeof(unsigned),
        "The buckets_scanned bitset is represented by an unsigned int and cannot support this many "
        "buckets on this platform.");
typedef struct {
    /* reported when each probe response is received, if report_events
     * enabled in wifi_scan_cmd_params. buckets_scanned is a bitset of the
     * buckets that are currently being scanned. See the buckets_scanned field
     * in the wifi_cached_scan_results struct for more details.
     */
    void (*on_full_scan_result) (wifi_request_id id, wifi_scan_result *result,
                                 unsigned buckets_scanned);

    /* indicates progress of scanning statemachine */
    void (*on_scan_event) (wifi_request_id id, wifi_scan_event event);

} wifi_scan_result_handler;

typedef struct {
    wifi_channel channel;               // frequency
    int dwellTimeMs;                    // dwell time hint
    int passive;                        // 0 => active, 1 => passive scan; ignored for DFS
    /* Add channel class */
} wifi_scan_channel_spec;

#define REPORT_EVENTS_EACH_SCAN        (1 << 0)
#define REPORT_EVENTS_FULL_RESULTS     (1 << 1)
#define REPORT_EVENTS_NO_BATCH         (1 << 2)

typedef struct {
    int bucket;                         // bucket index, 0 based
    wifi_band band;                     // when UNSPECIFIED, use channel list
    int period;                         // desired period, in millisecond; if this is too
                                        // low, the firmware should choose to generate results as
                                        // fast as it can instead of failing the command.
                                        // for exponential backoff bucket this is the min_period
    /* report_events semantics -
     *  This is a bit field; which defines following bits -
     *  REPORT_EVENTS_EACH_SCAN    => report a scan completion event after scan. If this is not set
     *                                 then scan completion events should be reported if
     *                                 report_threshold_percent or report_threshold_num_scans is
     *                                 reached.
     *  REPORT_EVENTS_FULL_RESULTS => forward scan results (beacons/probe responses + IEs)
     *                                 in real time to HAL, in addition to completion events
     *                                 Note: To keep backward compatibility, fire completion
     *                                 events regardless of REPORT_EVENTS_EACH_SCAN.
     *  REPORT_EVENTS_NO_BATCH     => controls if scans for this bucket should be placed in the
     *                                 history buffer
     */
    byte report_events;
    int max_period; // if max_period is non zero or different than period, then this bucket is
                    // an exponential backoff bucket and the scan period will grow exponentially
                    // as per formula: actual_period(N) = period * (base ^ (N/step_count))
                    // to a maximum period of max_period
    int base;       // for exponential back off bucket: multiplier: new_period=old_period*base
    int step_count; // for exponential back off bucket, number of scans to perform for a given
                    // period

    int num_channels;
    // channels to scan; these may include DFS channels
    // Note that a given channel may appear in multiple buckets
    wifi_scan_channel_spec channels[MAX_CHANNELS];
} wifi_scan_bucket_spec;

typedef struct {
    int base_period;                    // base timer period in ms
    int max_ap_per_scan;                // number of access points to store in each scan entry in
                                        // the BSSID/RSSI history buffer (keep the highest RSSI
                                        // access points)
    int report_threshold_percent;       // in %, when scan buffer is this much full, wake up apps
                                        // processor
    int report_threshold_num_scans;     // in number of scans, wake up AP after these many scans
    int num_buckets;
    wifi_scan_bucket_spec buckets[MAX_BUCKETS];
} wifi_scan_cmd_params;

/*
 * Start periodic GSCAN
 * When this is called all requested buckets should be scanned, starting the beginning of the cycle
 *
 * For example:
 * If there are two buckets specified
 *  - Bucket 1: period=10s
 *  - Bucket 2: period=20s
 *  - Bucket 3: period=30s
 * Then the following scans should occur
 *  - t=0  buckets 1, 2, and 3 are scanned
 *  - t=10 bucket 1 is scanned
 *  - t=20 bucket 1 and 2 are scanned
 *  - t=30 bucket 1 and 3 are scanned
 *  - t=40 bucket 1 and 2 are scanned
 *  - t=50 bucket 1 is scanned
 *  - t=60 buckets 1, 2, and 3 are scanned
 *  - and the patter repeats
 *
 * If any scan does not occur or is incomplete (error, interrupted, etc) then a cached scan result
 * should still be recorded with the WIFI_SCAN_FLAG_INTERRUPTED flag set.
 */
wifi_error wifi_start_gscan(wifi_request_id id, wifi_interface_handle iface,
        wifi_scan_cmd_params params, wifi_scan_result_handler handler);

/* Stop periodic GSCAN */
wifi_error wifi_stop_gscan(wifi_request_id id, wifi_interface_handle iface);

typedef enum {
    WIFI_SCAN_FLAG_INTERRUPTED = 1      // Indicates that scan results are not complete because
                                        // probes were not sent on some channels
} wifi_scan_flags;

/* Get the GSCAN cached scan results */
typedef struct {
    int scan_id;                                     // a unique identifier for the scan unit
    int flags;                                       // a bitmask with additional
                                                     // information about scan.
    unsigned buckets_scanned;                        // a bitset of the buckets that were scanned.
                                                     // for example a value of 13 (0b1101) would
                                                     // indicate that buckets 0, 2 and 3 were
                                                     // scanned to produce this list of results.
                                                     // should be set to 0 if this information is
                                                     // not available.
    int num_results;                                 // number of bssids retrieved by the scan
    wifi_scan_result results[MAX_AP_CACHE_PER_SCAN]; // scan results - one for each bssid
} wifi_cached_scan_results;

wifi_error wifi_get_cached_gscan_results(wifi_interface_handle iface, byte flush,
        int max, wifi_cached_scan_results *results, int *num);

/* BSSID Hotlist */
typedef struct {
    void (*on_hotlist_ap_found)(wifi_request_id id,
            unsigned num_results, wifi_scan_result *results);
    void (*on_hotlist_ap_lost)(wifi_request_id id,
            unsigned num_results, wifi_scan_result *results);
} wifi_hotlist_ap_found_handler;

typedef struct {
    mac_addr  bssid;                    // AP BSSID
    wifi_rssi low;                      // low threshold
    wifi_rssi high;                     // high threshold
} ap_threshold_param;

typedef struct {
    int lost_ap_sample_size;
    int num_bssid;                                 // number of hotlist APs
    ap_threshold_param ap[MAX_HOTLIST_APS];     // hotlist APs
} wifi_bssid_hotlist_params;

/* Set the BSSID Hotlist */
wifi_error wifi_set_bssid_hotlist(wifi_request_id id, wifi_interface_handle iface,
        wifi_bssid_hotlist_params params, wifi_hotlist_ap_found_handler handler);

/* Clear the BSSID Hotlist */
wifi_error wifi_reset_bssid_hotlist(wifi_request_id id, wifi_interface_handle iface);

/* SSID Hotlist */
typedef struct {
    void (*on_hotlist_ssid_found)(wifi_request_id id,
            unsigned num_results, wifi_scan_result *results);
    void (*on_hotlist_ssid_lost)(wifi_request_id id,
            unsigned num_results, wifi_scan_result *results);
} wifi_hotlist_ssid_handler;

typedef struct {
    char  ssid[32+1];                   // SSID
    wifi_band band;                     // band for this set of threshold params
    wifi_rssi low;                      // low threshold
    wifi_rssi high;                     // high threshold
} ssid_threshold_param;

typedef struct {
    int lost_ssid_sample_size;
    int num_ssid;                                   // number of hotlist SSIDs
    ssid_threshold_param ssid[MAX_HOTLIST_SSID];    // hotlist SSIDs
} wifi_ssid_hotlist_params;

/* BSSID blacklist */
typedef struct {
    int num_bssid;                           // number of blacklisted BSSIDs
    mac_addr bssids[MAX_BLACKLIST_BSSID];    // blacklisted BSSIDs
} wifi_bssid_params;

/* Set the BSSID blacklist */
wifi_error wifi_set_bssid_blacklist(wifi_request_id id, wifi_interface_handle iface,
        wifi_bssid_params params);

/* Significant wifi change */
typedef struct {
    mac_addr bssid;                     // BSSID
    wifi_channel channel;               // channel frequency in MHz
    int num_rssi;                       // number of rssi samples
    wifi_rssi rssi[];                   // RSSI history in db
} wifi_significant_change_result;

typedef struct {
    void (*on_significant_change)(wifi_request_id id,
            unsigned num_results, wifi_significant_change_result **results);
} wifi_significant_change_handler;

// The sample size parameters in the wifi_significant_change_params structure
// represent the number of occurence of a g-scan where the BSSID was seen and RSSI was
// collected for that BSSID, or, the BSSID was expected to be seen and didn't.
// for instance: lost_ap_sample_size : number of time a g-scan was performed on the
// channel the BSSID was seen last, and the BSSID was not seen during those g-scans
typedef struct {
    int rssi_sample_size;               // number of samples for averaging RSSI
    int lost_ap_sample_size;            // number of samples to confirm AP loss
    int min_breaching;                  // number of APs breaching threshold
    int num_bssid;                         // max 64
    ap_threshold_param ap[MAX_SIGNIFICANT_CHANGE_APS];
} wifi_significant_change_params;

/* Set the Signifcant AP change list */
wifi_error wifi_set_significant_change_handler(wifi_request_id id, wifi_interface_handle iface,
        wifi_significant_change_params params, wifi_significant_change_handler handler);

/* Clear the Signifcant AP change list */
wifi_error wifi_reset_significant_change_handler(wifi_request_id id, wifi_interface_handle iface);

/* Random MAC OUI for PNO */
wifi_error wifi_set_scanning_mac_oui(wifi_interface_handle handle, oui scan_oui);


// Enhanced PNO:
// Enhanced PNO feature is expected to be enabled all of the time (e.g. screen lit) and may thus
// require firmware to store a large number of networks, covering the whole list of known networks.
// Therefore, it is acceptable for firmware to store a crc24, crc32 or other short hash of the SSID,
// such that a low but non-zero probability of collision exist. With that scheme it should be
// possible for firmware to keep an entry as small as 4 bytes for each pno network.
// For instance, a firmware pn0 entry can be implemented in the form of:
//          PNO ENTRY = crc24(3 bytes) | flags>>3 (5 bits) | auth flags(3 bits)
//
// No scans should be automatically performed by the chip. Instead all scan results from gscan
// should be scored and the wifi_epno_handler on_network_found callback should be called with
// the scan results.
//
// A PNO network shall be reported once, that is, once a network is reported by firmware
// its entry shall be marked as "done" until framework calls wifi_set_epno_list again.
// Calling wifi_set_epno_list shall reset the "done" status of pno networks in firmware.
//
// A network should only be considered found if its RSSI is above the minimum RSSI for its
// frequency range (min5GHz_rssi and min24GHz_rssi for 5GHz and 2.4GHz networks respectively).
// When disconnected the list of scan results should be returned if any network is found.
// When connected the scan results shall be reported only if the score of any network in the scan
// is greater than that of the currently connected BSSID.
//
// The FW should calculate the score of all the candidates (including currently connected one)
//   with following equation:
//     RSSI score = (RSSI + 85) * 4;
//     If RSSI score > initial_score_max , RSSI score = initial_score_max;
//     final score = RSSI score
//         + current_connection_bonus (if currently connected BSSID)
//         + same_network_bonus (if network has SAME_NETWORK flag)
//         + secure_bonus (if the network is not open)
//         + band5GHz_bonus (if BSSID is on 5G)
//     If there is a BSSID’s score > current BSSID’s score, then report the cached scan results
//         at the end of the scan (excluding the ones on blacklist) to the upper layer.
// Additionally, all BSSIDs that are in the BSSID blacklist should be ignored by Enhanced PNO

// Whether directed scan needs to be performed (for hidden SSIDs)
#define WIFI_PNO_FLAG_DIRECTED_SCAN (1 << 0)
// Whether PNO event shall be triggered if the network is found on A band
#define WIFI_PNO_FLAG_A_BAND (1 << 1)
// Whether PNO event shall be triggered if the network is found on G band
#define WIFI_PNO_FLAG_G_BAND (1 << 2)
// Whether strict matching is required
// If required then the firmware must store the network's SSID and not just a hash
#define WIFI_PNO_FLAG_STRICT_MATCH (1 << 3)
// If this SSID should be considered the same network as the currently connected one for scoring
#define WIFI_PNO_FLAG_SAME_NETWORK (1 << 4)

// Code for matching the beacon AUTH IE - additional codes TBD
#define WIFI_PNO_AUTH_CODE_OPEN  (1 << 0) // open
#define WIFI_PNO_AUTH_CODE_PSK   (1 << 1) // WPA_PSK or WPA2PSK
#define WIFI_PNO_AUTH_CODE_EAPOL (1 << 2) // any EAPOL

typedef struct {
    char ssid[32+1];     // null terminated
    byte flags;          // WIFI_PNO_FLAG_XXX
    byte auth_bit_field; // auth bit field for matching WPA IE
} wifi_epno_network;

/* ePNO Parameters */
typedef struct {
    int min5GHz_rssi;               // minimum 5GHz RSSI for a BSSID to be considered
    int min24GHz_rssi;              // minimum 2.4GHz RSSI for a BSSID to be considered
    int initial_score_max;          // the maximum score that a network can have before bonuses
    int current_connection_bonus;   // only report when there is a network's score this much higher
                                    // than the current connection.
    int same_network_bonus;         // score bonus for all networks with the same network flag
    int secure_bonus;               // score bonus for networks that are not open
    int band5GHz_bonus;             // 5GHz RSSI score bonus (applied to all 5GHz networks)
    int num_networks;               // number of wifi_epno_network objects
    wifi_epno_network networks[MAX_EPNO_NETWORKS];   // PNO networks
} wifi_epno_params;

typedef struct {
    // on results
    void (*on_network_found)(wifi_request_id id,
            unsigned num_results, wifi_scan_result *results);
} wifi_epno_handler;


/* Set the ePNO list - enable ePNO with the given parameters */
wifi_error wifi_set_epno_list(wifi_request_id id, wifi_interface_handle iface,
        const wifi_epno_params *epno_params, wifi_epno_handler handler);

/* Reset the ePNO list - no ePNO networks should be matched after this */
wifi_error wifi_reset_epno_list(wifi_request_id id, wifi_interface_handle iface);


typedef struct {
    int  id;                            // identifier of this network block, report this in event
    char realm[256];                    // null terminated UTF8 encoded realm, 0 if unspecified
    int64_t roamingConsortiumIds[16];   // roaming consortium ids to match, 0s if unspecified
    byte plmn[3];                       // mcc/mnc combination as per rules, 0s if unspecified
} wifi_passpoint_network;

typedef struct {
    void (*on_passpoint_network_found)(
            wifi_request_id id,
            int net_id,                        // network block identifier for the matched network
            wifi_scan_result *result,          // scan result, with channel and beacon information
            int anqp_len,                      // length of ANQP blob
            byte *anqp                         // ANQP data, in the information_element format
            );
} wifi_passpoint_event_handler;

/* Sets a list for passpoint networks for PNO purposes; it should be matched
 * against any passpoint networks (designated by Interworking element) found
 * during regular PNO scan. */
wifi_error wifi_set_passpoint_list(wifi_request_id id, wifi_interface_handle iface, int num,
        wifi_passpoint_network *networks, wifi_passpoint_event_handler handler);

/* Reset passpoint network list - no Passpoint networks should be matched after this */
wifi_error wifi_reset_passpoint_list(wifi_request_id id, wifi_interface_handle iface);

#endif
