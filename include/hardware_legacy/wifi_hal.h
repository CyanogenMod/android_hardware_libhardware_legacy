/*
 * Copyright (C) 2014 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef __WIFI_HAL_H__
#define __WIFI_HAL_H__

#ifdef __cplusplus
extern "C"
{
#endif
#include <stdint.h>

/* WiFi Common definitions */
/* channel operating width */
typedef enum {
    WIFI_CHAN_WIDTH_20    = 0,
    WIFI_CHAN_WIDTH_40    = 1,
    WIFI_CHAN_WIDTH_80    = 2,
    WIFI_CHAN_WIDTH_160   = 3,
    WIFI_CHAN_WIDTH_80P80 = 4,
    WIFI_CHAN_WIDTH_5     = 5,
    WIFI_CHAN_WIDTH_10    = 6,
    WIFI_CHAN_WIDTH_INVALID = -1
} wifi_channel_width;

typedef int wifi_radio;
typedef int wifi_channel;

typedef struct {
    wifi_channel_width width;
    int center_frequency0;
    int center_frequency1;
    int primary_frequency;
} wifi_channel_spec;

typedef enum {
    WIFI_SUCCESS = 0,
    WIFI_ERROR_NONE = 0,
    WIFI_ERROR_UNKNOWN = -1,
    WIFI_ERROR_UNINITIALIZED = -2,
    WIFI_ERROR_NOT_SUPPORTED = -3,
    WIFI_ERROR_NOT_AVAILABLE = -4,              // Not available right now, but try later
    WIFI_ERROR_INVALID_ARGS = -5,
    WIFI_ERROR_INVALID_REQUEST_ID = -6,
    WIFI_ERROR_TIMED_OUT = -7,
    WIFI_ERROR_TOO_MANY_REQUESTS = -8,          // Too many instances of this request
    WIFI_ERROR_OUT_OF_MEMORY = -9
} wifi_error;

typedef unsigned char byte;
typedef unsigned char u8;
typedef signed char s8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef int32_t s32;
typedef uint64_t u64;
typedef int64_t s64;
typedef int wifi_request_id;
typedef int wifi_channel;                       // indicates channel frequency in MHz
typedef int wifi_rssi;
typedef byte mac_addr[6];
typedef byte oui[3];
typedef int64_t wifi_timestamp;                 // In microseconds (us)
typedef int64_t wifi_timespan;                  // In nanoseconds  (ns)

typedef struct wifi_info *wifi_handle;
typedef struct wifi_interface_info *wifi_interface_handle;

/* Initialize/Cleanup */

wifi_error wifi_initialize(wifi_handle *handle);
typedef void (*wifi_cleaned_up_handler) (wifi_handle handle);
void wifi_cleanup(wifi_handle handle, wifi_cleaned_up_handler handler);
void wifi_event_loop(wifi_handle handle);

/* Error handling */
void wifi_get_error_info(wifi_error err, const char **msg); // return a pointer to a static string

/* Feature enums */
#define WIFI_FEATURE_INFRA              0x0001      // Basic infrastructure mode
#define WIFI_FEATURE_INFRA_5G           0x0002      // Support for 5 GHz Band
#define WIFI_FEATURE_HOTSPOT            0x0004      // Support for GAS/ANQP
#define WIFI_FEATURE_P2P                0x0008      // Wifi-Direct
#define WIFI_FEATURE_SOFT_AP            0x0010      // Soft AP
#define WIFI_FEATURE_GSCAN              0x0020      // Google-Scan APIs
#define WIFI_FEATURE_NAN                0x0040      // Neighbor Awareness Networking
#define WIFI_FEATURE_D2D_RTT            0x0080      // Device-to-device RTT
#define WIFI_FEATURE_D2AP_RTT           0x0100      // Device-to-AP RTT
#define WIFI_FEATURE_BATCH_SCAN         0x0200      // Batched Scan (legacy)
#define WIFI_FEATURE_PNO                0x0400      // Preferred network offload
#define WIFI_FEATURE_ADDITIONAL_STA     0x0800      // Support for two STAs
#define WIFI_FEATURE_TDLS               0x1000      // Tunnel directed link setup
#define WIFI_FEATURE_TDLS_OFFCHANNEL    0x2000      // Support for TDLS off channel
#define WIFI_FEATURE_EPR                0x4000      // Enhanced power reporting
#define WIFI_FEATURE_AP_STA             0x8000      // Support for AP STA Concurrency
#define WIFI_FEATURE_LINK_LAYER_STATS  0x10000      // Link layer stats collection
#define WIFI_FEATURE_LOGGER            0x20000      // WiFi Logger
#define WIFI_FEATURE_HAL_EPNO          0x40000      // WiFi PNO enhanced

// Add more features here


typedef int feature_set;

#define IS_MASK_SET(mask, flags)        ((flags & mask) == mask)
#define IS_MASK_RESET(mask, flags)      ((flags & mask) == 0)

#define IS_SUPPORTED_FEATURE(feature, featureSet)       IS_MASK_SET(feature, fetureSet)
#define IS_UNSUPPORTED_FEATURE(feature, featureSet)     IS_MASK_RESET(feature, fetureSet)

/* Feature set */
wifi_error wifi_get_supported_feature_set(wifi_interface_handle handle, feature_set *set);

/*
 * Each row represents a valid feature combination;
 * all other combinations are invalid!
 */
wifi_error wifi_get_concurrency_matrix(wifi_interface_handle handle, int set_size_max,
        feature_set set[], int *set_size);

/* multiple interface support */

wifi_error wifi_get_ifaces(wifi_handle handle, int *num_ifaces, wifi_interface_handle **ifaces);
wifi_error wifi_get_iface_name(wifi_interface_handle iface, char *name, size_t size);

/* Configuration events */

typedef struct {
    void (*on_country_code_changed)(char code[2]);      // We can get this from supplicant too

    // More event handlers
} wifi_event_handler;

wifi_error wifi_set_iface_event_handler(wifi_request_id id, wifi_interface_handle iface, wifi_event_handler eh);
wifi_error wifi_reset_iface_event_handler(wifi_request_id id, wifi_interface_handle iface);

wifi_error wifi_set_nodfs_flag(wifi_interface_handle handle, u32 nodfs);

/* include various feature headers */

#include "gscan.h"
#include "link_layer_stats.h"
#include "rtt.h"
#include "tdls.h"
#include "wifi_logger.h"
#include "wifi_config.h"
#include "wifi_nan.h"
//wifi HAL function pointer table
typedef struct {
    wifi_error (* wifi_initialize) (wifi_handle *);
    void (* wifi_cleanup) (wifi_handle, wifi_cleaned_up_handler);
    void (*wifi_event_loop)(wifi_handle);
    void (* wifi_get_error_info) (wifi_error , const char **);
    wifi_error (* wifi_get_supported_feature_set) (wifi_interface_handle, feature_set *);
    wifi_error (* wifi_get_concurrency_matrix) (wifi_interface_handle, int, feature_set *, int *);
    wifi_error (* wifi_set_scanning_mac_oui) (wifi_interface_handle, unsigned char *);
    wifi_error (* wifi_get_supported_channels)(wifi_handle, int *, wifi_channel *);
    wifi_error (* wifi_is_epr_supported)(wifi_handle);
    wifi_error (* wifi_get_ifaces) (wifi_handle , int *, wifi_interface_handle **);
    wifi_error (* wifi_get_iface_name) (wifi_interface_handle, char *name, size_t);
    wifi_error (* wifi_set_iface_event_handler) (wifi_request_id,wifi_interface_handle ,
            wifi_event_handler);
    wifi_error (* wifi_reset_iface_event_handler) (wifi_request_id, wifi_interface_handle);
    wifi_error (* wifi_start_gscan) (wifi_request_id, wifi_interface_handle, wifi_scan_cmd_params,
            wifi_scan_result_handler);
    wifi_error (* wifi_stop_gscan)(wifi_request_id, wifi_interface_handle);
    wifi_error (* wifi_get_cached_gscan_results)(wifi_interface_handle, byte, int,
            wifi_cached_scan_results *, int *);
    wifi_error (* wifi_set_bssid_hotlist)(wifi_request_id, wifi_interface_handle,
            wifi_bssid_hotlist_params, wifi_hotlist_ap_found_handler);
    wifi_error (* wifi_reset_bssid_hotlist)(wifi_request_id, wifi_interface_handle);
    wifi_error (* wifi_set_significant_change_handler)(wifi_request_id, wifi_interface_handle,
            wifi_significant_change_params, wifi_significant_change_handler);
    wifi_error (* wifi_reset_significant_change_handler)(wifi_request_id, wifi_interface_handle);
    wifi_error (* wifi_get_gscan_capabilities)(wifi_interface_handle, wifi_gscan_capabilities *);
    wifi_error (* wifi_set_link_stats) (wifi_interface_handle, wifi_link_layer_params);
    wifi_error (* wifi_get_link_stats) (wifi_request_id,wifi_interface_handle,
            wifi_stats_result_handler);
    wifi_error (* wifi_clear_link_stats)(wifi_interface_handle,u32, u32 *, u8, u8 *);
    wifi_error (* wifi_get_valid_channels)(wifi_interface_handle,int, int, wifi_channel *, int *);
    wifi_error (* wifi_rtt_range_request)(wifi_request_id, wifi_interface_handle, unsigned,
            wifi_rtt_config[], wifi_rtt_event_handler);
    wifi_error (* wifi_rtt_range_cancel)(wifi_request_id,  wifi_interface_handle, unsigned,
            mac_addr[]);
    wifi_error (* wifi_get_rtt_capabilities)(wifi_interface_handle, wifi_rtt_capabilities *);
    wifi_error (* wifi_set_nodfs_flag)(wifi_interface_handle, u32);
    wifi_error (* wifi_start_logging)(wifi_interface_handle, u32, u32, u32, u32, char *);
    wifi_error (* wifi_set_epno_list)(int, wifi_interface_info *, int, wifi_epno_network *,
            wifi_epno_handler);
    wifi_error (* wifi_set_country_code)(wifi_interface_handle, const char *);
} wifi_hal_fn;
wifi_error init_wifi_vendor_hal_func_table(wifi_hal_fn *fn);
#ifdef __cplusplus
}
#endif

#endif

