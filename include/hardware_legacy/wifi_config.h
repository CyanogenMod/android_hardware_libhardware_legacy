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
wifi_error wifi_extended_dtim_config_set(wifi_request_id id,
        wifi_interface_handle iface, int extended_dtim);


#ifdef __cplusplus
}

#endif /* __cplusplus */

#endif /*__WIFI_HAL_STATS_ */

