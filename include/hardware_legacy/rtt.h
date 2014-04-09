
#include "wifi_hal.h"

#ifndef __WIFI_HAL_RTT_H__
#define __WIFI_HAL_RTT_H__

/* RTT */

/* Type */

#define RTT_TYPE_RTS_CTS    1
#define RTT_TYPE_NULL_ACK   2
#define RTT_TYPE_3          3                   // This is required for D2D RTT

typedef struct {
    int type;                                   // One of RTT_TYPE_ values
    byte token[16];                             // Local token to be published in rtt service info
    wifi_channel primary;                       // Use primary when it doesn't conflict with infra
    wifi_channel secondary;                     // Use secondary when primary conflicts with infra
    wifi_timespan offset_from_dw_timestamp;     // Time offset to go to primary or secondary channel
    char rtt_service_name[32];                  // Service name to use
    byte rtt_servic_info[256];                  // Blob to publish in service info
} RttConfig;

typedef struct {
    void (*on_device_ranged)(wifi_request_id id, mac_addr addr, byte token[16], wifi_timespan ts);
    void (*on_failed)(wifi_request_id id, wifi_error reason);
} wifi_rtt_event_handler;

/* Enable rtt - publishes a service advertising RTT; and starts playing
 * two phase nXn RTT protocol. To stop it, use wifi_disable_rtt */

wifi_error wifi_enable_rtt(wifi_request_id id, wifi_interface_handle iface, RttConfig config,
        wifi_rtt_event_handler handler);
wifi_error wifi_disable_rtt(wifi_request_id id);

/* RTT Hotlist */

typedef struct {
    byte token[16];
    int64_t max_rtt_timespan;
} wifi_rtt_hotlist_criterion;

typedef struct {
    void (*on_token_found)(wifi_request_id id, byte token[16], wifi_timespan ts);
} wifi_rtt_hotlist_event_handler;

wifi_error wifi_set_rtt_hotlist(wifi_request_id id, wifi_interface_handle iface, int num_criteria,
        wifi_rtt_hotlist_criterion *criteria, wifi_rtt_hotlist_event_handler handler);
wifi_error wifi_reset_rtt_hotlist(wifi_request_id id);

/* single shot RTT (Device to AP??) */

typedef struct {
    void (*on_succeeded) (wifi_request_id id, wifi_timestamp ts);
    void (*on_failed) (wifi_request_id id, wifi_error reason);
} IRttEventHandler;

wifi_error wifi_rtt_request_range(wifi_request_id id, wifi_interface_handle iface, mac_addr addr,
        RttConfig config, int continuous, IRttEventHandler *handler);
void wifi_rtt_cancel_range_request(wifi_request_id id);

#endif

