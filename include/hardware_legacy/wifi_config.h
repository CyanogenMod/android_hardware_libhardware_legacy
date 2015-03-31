#include "wifi_hal.h"

#ifndef __WIFI_HAL_CONFIG_H
#define __WIFI_HAL_CONFIG_H

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

#define CONFIG_MAJOR_VERSION      1
#define CONFIG_MINOR_VERSION      0
#define CONFIG_MICRO_VERSION      0

typedef int wifi_radio;

// whether the wifi chipset wakes at every dtim beacon or a multiple of the dtim period
// if extended_dtim is set to 3, the STA shall wake up every 3 DTIM beacons
wifi_error wifi_extended_dtim_config_set(wifi_request_id id,
        wifi_interface_handle iface, int extended_dtim);

//set the country code to driver
wifi_error wifi_set_country_code(wifi_interface_handle iface, const char* country_code);

//set the wifi_iface stats averaging factor used to calculate
// statistics like average the TSF offset or average number of frame leaked
// For instance, upon beacon reception:
//    current_avg = ((beacon_TSF - TBTT) * factor + previous_avg * (0x10000 - factor) ) / 0x10000
// For instance, when evaluating leaky APs:
//    current_avg = ((num frame received within guard time) * factor + previous_avg * (0x10000 - factor)) / 0x10000

wifi_error wifi_set_beacon_wifi_iface_stats_averaging_factor(wifi_request_id id, wifi_interface_handle iface,
        u16 factor);

// configure guard time, i.e. when implementing IEEE power management based on
// frame control PM bit, how long driver waits before shutting down the radio and
// after receiving an ACK for a data frame with PM bit set
wifi_error wifi_set_guard_time(wifi_request_id id, wifi_interface_handle iface,
        u32 guard_time);

#ifdef __cplusplus
}

#endif /* __cplusplus */

#endif /*__WIFI_HAL_STATS_ */

