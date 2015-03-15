#include "wifi_hal.h"

#ifndef __WIFI_HAL_LOGGER_H
#define __WIFI_HAL_LOGGER_H

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

#define LOGGER_MAJOR_VERSION      1
#define LOGGER_MINOR_VERSION      0
#define LOGGER_MICRO_VERSION      0

typedef int wifi_radio;
typedef int wifi_ring_buffer_id;

/**
 * This structure represent a logger entry within a ring.
 * Binary entries can be used so as to store packet data or vendor specific information.
 */
typedef struct {
    u16 entry_size:13;
    u16 binary:1; //set for binary entries
    u16 has_timestamp:1; //set if 64 bits timestamp is present
    u16 reserved:1;
    u8 type; // Per ring specific
    u8 resvd;
    u64 timestamp; //present if has_timestamp bit is set.
    u8 data[0];
} wifi_ring_buffer_entry;

#define WIFI_RING_BUFFER_FLAG_HAS_BINARY_ENTRIES         0x00000001     // set if binary entries are present
#define WIFI_RING_BUFFER_FLAG_HAS_ASCII_ENTRIES          0x00000002     // set if ascii entries are present

/* ring buffer params */
/**
 * written_bytes and read_bytes implement a producer consumer API
 *     hence written_bytes >= read_bytes
 * a modulo arithmetic of the buffer size has to be applied to those counters:
 * actual offset into ring buffer = written_bytes % ring_buffer_byte_size
 *
 */
typedef struct {
   u8 name[32];
   u32 flags;
   u64 fd; // linux file descriptor for that buffer
   u32 ring_buffer_byte_size;   // total memory size allocated for the buffer
   u32 verbose_level; //
   u32 written_bytes; // number of bytes that was written to the buffer by driver, monotonously increasing integer
   u32 read_bytes;  // number of bytes that was read from the buffer by user land, monotonously increasing integer
} wifi_ring_buffer_status;

/* API to trigger the debug collection.
   Unless his API is invoked - logging is not triggered.
   - verbose_level 0 corresponds to minimal or no collection
   - verbose_level 1+ are TBD
   */
wifi_error wifi_start_logging(wifi_interface_handle iface, u32 verbose_level, u8 * buffer_name);

/* callback for reporting ring buffer status */
typedef struct {
  void (*on_ring_buffer_status_results) (wifi_request_id id, u32 num_buffers, wifi_ring_buffer_status *status);
} wifi_ring_buffer_status_result_handler;

/* api to get the status of a ring buffer */
wifi_error wifi_get_ring_buffer_status(wifi_request_id id,
        wifi_interface_handle iface, wifi_ring_buffer_id ring_id, wifi_ring_buffer_status_result_handler handler);

/* api to collect a firmware memory dump for a given iface */
wifi_error wifi_get_firmware_memory_dump(wifi_request_id id,
        wifi_interface_handle iface, char * buffer, int buffer_size);


/* Feature set */
#define WIFI_LOGGER_MEMORY_DUMP_SUPPORTED 1
#define WIFI_LOGGER_PER_PACKET_TX_RX_STATUS_SUPPORTED 2

wifi_error wifi_get_logger_supported_feature_set(wifi_interface_handle handle, unsigned int *support);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /*__WIFI_HAL_STATS_ */

