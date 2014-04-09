
#include "wifi_hal.h"

#ifndef __WIFI_HAL_GSCAN_H__
#define __WIFI_HAL_GSCAN_H__

/* AP Scans */

typedef enum {
    WIFI_BAND_UNSPECIFIED,
    WIFI_BAND_BG,                       // 2.4 GHz
    WIFI_BAND_A,                        // 5 GHz without DFS
    WIFI_BAND_A_WITH_DFS,               // 5 GHz with DFS
    WIFI_BAND_ABG,                      // 2.4 GHz + 5 GHz; no DFS
    WIFI_BAND_ABG_WITH_DFS,             // 2.4 GHz + 5 GHz with DFS
} wifi_band;

wifi_error wifi_get_valid_channels(wifi_interface_handle handle,
        int band, int size, wifi_channel *channels, int *num);

typedef struct {
    int max_scan_cache_size;                 // in number of scan results??
    int max_scan_buckets;
    int max_ap_cache_per_scan;
    int max_rssi_sample_size;
    int max_scan_reporting_threshold;        // in number of scan results??
    int max_hotlist_aps;
    int max_significant_wifi_change_aps;
} wifi_gscan_capabilities;

wifi_error wifi_get_gscan_capabilities(wifi_interface_handle handle,
        wifi_gscan_capabilities *capabilities);

typedef struct {
    wifi_timestamp ts;                  // Time of discovery
    char ssid[32+1];                    // null terminated
    mac_addr bssid;
    wifi_channel channel;               // channel frequency in MHz
    wifi_rssi rssi;                     // in db
    wifi_timespan rtt;                  // in nanoseconds
    wifi_timespan rtt_sd;               // standard deviation in rtt

    // other fields
} wifi_scan_result;

typedef struct {
    void (*on_scan_results_available) (wifi_request_id id, unsigned num_results_available);
} wifi_scan_result_handler;

typedef struct {
    wifi_channel channel;               // frequency
    int dwellTimeMs;                    // dwell time hint
    int passive;                        // 0 => active, 1 => passive scan; ignored for DFS
    /* Add channel class */
} wifi_scan_channel_spec;

typedef struct {
    int bucket;                         // bucket index, 0 based
    wifi_band band;                     // when UNSPECIFIED, use channel list
    int period;                         // desired period, in millisecond; if this is too
                                        // low, the firmware should choose to generate results as
                                        // fast as it can instead of failing the command
    byte report_events;                 // 1 => report events after each scan
    int num_channels;
    wifi_scan_channel_spec channels[8]; // channels to scan; these may include DFS channels
} wifi_scan_bucket_spec;

typedef struct {
    int base_period;                    // base timer period in ms
    int max_ap_per_scan;
    int report_threshold;               // in %, when buffer is this much full, wake up AP
    int num_buckets;                    // maximum 8
    wifi_scan_bucket_spec buckets[8];
} wifi_scan_cmd_params;

wifi_error wifi_start_gscan(wifi_request_id id, wifi_interface_handle iface,
        wifi_scan_cmd_params params, wifi_scan_result_handler handler);
wifi_error wifi_stop_gscan(wifi_request_id id, wifi_interface_handle iface);

wifi_error wifi_get_cached_gscan_results(wifi_interface_handle iface, byte flush,
        wifi_scan_result *results, int *num);


/* BSSID Hotlist */
typedef struct {
    void (*on_hotlist_ap_found)(wifi_request_id id,
            unsigned num_results, wifi_scan_result *results);
} wifi_hotlist_ap_found_handler;

typedef struct {
    mac_addr  bssid;                    // AP BSSID
    wifi_rssi low;                      // low threshold
    wifi_rssi high;                     // high threshold
} ap_threshold_param;

typedef struct {
    int num;                            // max??
    ap_threshold_param bssids[64];
} wifi_bssid_hotlist_params;

wifi_error wifi_set_bssid_hotlist(wifi_request_id id, wifi_interface_handle iface,
        wifi_bssid_hotlist_params params, wifi_hotlist_ap_found_handler handler);
wifi_error wifi_reset_bssid_hotlist(wifi_request_id id, wifi_interface_handle iface);

/* Significant wifi change*/

typedef struct {
    void (*on_significant_change)(wifi_request_id id,
            unsigned num_results, wifi_scan_result *results);
} wifi_significant_change_handler;

typedef struct {
    int rssi_sample_size;               // number of samples for averaging RSSI
    int lost_ap_sample_size;            // number of samples to confirm AP loss
    int min_breaching;                  // number of APs breaching threshold
    int num;                            // max 64
    ap_threshold_param bssids[64];
} wifi_significant_change_params;

wifi_error wifi_set_significant_change_handler(wifi_request_id id, wifi_interface_handle iface,
        wifi_significant_change_params params, wifi_significant_change_handler handler);
wifi_error wifi_reset_significant_change_handler(wifi_request_id id, wifi_interface_handle iface);

#endif

