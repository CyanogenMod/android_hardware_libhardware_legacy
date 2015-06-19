#include "wifi_hal.h"

#ifndef __WIFI_HAL_OFFLOAD_H
#define __WIFI_HAL_OFFLOAD_H

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

#define ETHER_ADDR_LEN		6	// Ethernet frame address length
#define N_AVAIL_ID		3	// available mkeep_alive IDs from 1 to 3
#define MKEEP_ALIVE_IP_PKT_MAX	256     // max size of IP packet for keep alive

/**
 * Send specified keep alive packet periodically.
 */
wifi_error wifi_start_sending_offloaded_packet(wifi_request_id id, wifi_interface_handle iface,
        u8 *ip_packet, u16 ip_packet_len, u8 *src_mac_addr, u8 *dst_mac_addr, u32 period_msec);

/**
 * Stop sending keep alive packet.
 */
wifi_error wifi_stop_sending_offloaded_packet(wifi_request_id id, wifi_interface_handle iface);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /*__WIFI_HAL_OFFLOAD_H */
