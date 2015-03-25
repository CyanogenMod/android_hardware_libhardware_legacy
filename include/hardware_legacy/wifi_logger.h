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



/**
 * WiFi logger life cycle is as follow:
 *
 * - at initialization time, framework will call wifi_get_ring_buffers_status so as to obtain the
 * names and list of supported buffers
 * - when WiFi operation start framework will call wifi_start_logging so as to trigger log collection
 * - Developper UI will provide an option to the user, so as it can set the verbose level of individual buffer
 *    as reported by wifi_get_ring_buffers_status
 * - during wifi operations, driver will periodically report per ring data to framework by invoking the
 *    on_ring_buffer_data call back
 * - when capturing a bug report, framework will indicate to driver that all the data has to be uploaded,
 *   urgently, by calling wifi_get_ring_data
 *
 * The data uploaded by driver will be stored by framework in separate files, with one stream of file per ring.
 * Framework will store the files in pcapng format, allowing for easy merging and parsing with network
 * analyzer tools.
 */


typedef int wifi_radio;
typedef int wifi_ring_buffer_id;

#define PER_PACKET_ENTRY_FLAGS_DIRECTION_TX 1 //  0: TX, 1: RX
#define PER_PACKET_ENTRY_FLAGS_TX_SUCCESS 2 // whether packet was transmitted or received/decrypted successfully
#define PER_PACKET_ENTRY_FLAGS_80211_HEADER 4 // has full 802.11 header, else has 802.3 header
#define PER_PACKET_ENTRY_FLAGS_PROTECTED 8  // whether packet was encrypted

typedef struct {
    u8 flags;
    u8 tid; // transmit or received tid
    u16 MCS; // modulation and bandwidth
    u8 rssi; // TX: RSSI of ACK for that packet
             // RX: RSSI of packet
    u8 num_retries; // number of attempted retries
    u16 last_transmit_rate; // last transmit rate in .5 mbps
    u16 link_layer_transmit_sequence; // transmit/reeive sequence for that MPDU packet
    u64 firmware_entry_timestamp;  // TX: firmware timestamp (us) when packet is queued within firmware buffer
                                   // for SDIO/HSIC or into PCIe buffer
                                   // RX : firmware receive timestamp
    u64 start_contention_timestamp; // firmware timestamp (us) when packet start contending for the
                                    // medium for the first time, at head of its AC queue,
                                    // or as part of an MPDU or A-MPDU. This timestamp is not updated
                                    // for each retry, only the first transmit attempt.
    u64 transmit_success_timestamp; // fimrware timestamp (us) when packet is successfully transmitted
                                    // or aborted because it has exhausted its maximum number of retries
    u8 data[0]; // packet data. The length of packet data is determined by the entry_size field of
                // the wifi_ring_buffer_entry structure. It is expected that first bytes of the
                // packet, or packet headers only (up to TCP or RTP/UDP headers) will be copied into the ring
} __attribute__((packed)) wifi_ring_per_packet_status_entry;

static char per_packet_status_ring_name[] = "wifi_per_packet_status"; // Ring buffer name for per-packet status ring

// Below events refer to the wifi_connectivity_event ring and shall be supported

#define WIFI_EVENT_ASSOCIATION_REQUESTED 0 // driver receive association command from kernel
#define WIFI_EVENT_AUTH_COMPLETE 1
#define WIFI_EVENT_ASSOC_COMPLETE 2
#define WIFI_EVENT_FW_AUTH_STARTED 3 // received firmware event indicating auth frames are sent
#define WIFI_EVENT_FW_ASSOC_STARTED 4 // received firmware event indicating assoc frames are sent
#define WIFI_EVENT_FW_RE_ASSOC_STARTED 5 // received firmware event indicating reassoc frames are sent
#define WIFI_EVENT_DRIVER_SCAN_REQUESTED 6
#define WIFI_EVENT_DRIVER_SCAN_RESULT_FOUND 7
#define WIFI_EVENT_DRIVER_SCAN_COMPLETE 8
#define WIFI_EVENT_G_SCAN_STARTED 9
#define WIFI_EVENT_G_SCAN_COMPLETE 10
#define WIFI_EVENT_DISASSOCIATION_REQUESTED 11
#define WIFI_EVENT_RE_ASSOCIATION_REQUESTED 12
#define WIFI_EVENT_ROAM_REQUESTED 13
#define WIFI_EVENT_BEACON_RECEIVED 14 // received beacon from AP (event enabled only in verbose mode)
#define WIFI_EVENT_ROAM_SCAN_STARTED 15 // firmware has triggered a roam scan (not g-scan)
#define WIFI_EVENT_ROAM_SCAN_COMPLETE 16 // firmware has completed a roam scan (not g-scan)
#define WIFI_EVENT_ROAM_SEARCH_STARTED 17 // firmware has started searching for roam candidates (with reason =xx)
#define WIFI_EVENT_ROAM_SEARCH_STOPPED 18 // firmware has stopped searching for roam candidates (with reason =xx)
#define WIFI_EVENT_CHANNEL_SWITCH_ANOUNCEMENT 20 // received channel switch anouncement from AP
#define WIFI_EVENT_FW_EAPOL_FRAME_TRANSMIT_START 21 // fw start transmit eapol frame, with EAPOL index 1-4
#define WIFI_EVENT_FW_EAPOL_FRAME_TRANSMIT_STOP 22 // fw gives up eapol frame, with rate, success/failure and number retries
#define WIFI_EVENT_DRIVER_EAPOL_FRAME_TRANSMIT_REQUESTED 23 // kernel queue EAPOL for transmission in tdriver
                                                            // with EAPOL index 1-4
#define WIFI_EVENT_FW_EAPOL_FRAME_RECEIVED 24 // with rate, regardless of the fact that EAPOL frame
                                           // is accepted or rejected by firmware
#define WIFI_EVENT_DRIVER_EAPOL_FRAME_RECEIVED 26 // with rate, and eapol index, driver has received
                                                  // EAPOL frame and will queue it up to wpa_supplicant
#define WIFI_EVENT_BLOCK_ACK_NEGOTIATION_COMPLETE 27 // with success/failure, parameters
#define WIFI_EVENT_BT_COEX_BT_SCO_START 28
#define WIFI_EVENT_BT_COEX_BT_SCO_STOP 29
#define WIFI_EVENT_BT_COEX_BT_SCAN_START 30 // for paging/scan etc..., when BT starts transmiting
                                            //    twice per BT slot
#define WIFI_EVENT_BT_COEX_BT_SCAN_STOP 31
#define WIFI_EVENT_BT_COEX_BT_HID_START 32
#define WIFI_EVENT_BT_COEX_BT_HID_STOP 33
#define WIFI_EVENT_ROAM_AUTH_STARTED 34 // firmware sends auth frame in roaming to next candidate
#define WIFI_EVENT_ROAM_AUTH_COMPLETE 35 // firmware receive auth confirm from ap
#define WIFI_EVENT_ROAM_ASSOC_STARTED 36 // firmware sends assoc/reassoc frame in
                                         //     roaming to next candidate
#define WIFI_EVENT_ROAM_ASSOC_COMPLETE 37 // firmware receive assoc/reassoc confirm from ap



// Parameters of wifi logger events are TLVs
// Event parameters tags are defined as:
#define WIFI_TAG_VENDOR_SPECIFIC 0 // take a byte stream as parameter
#define WIFI_TAG_BSSID 1 // takes a 6 bytes MAC address as parameter
#define WIFI_TAG_ADDR 2 // takes a 6 bytes MAC address as parameter
#define WIFI_TAG_SSID 3 // takes a 32 bytes SSID address as parameter
#define WIFI_TAG_STATUS 4 // takes an integer as parameter
#define WIFI_TAG_CHANNEL_SPEC 5 // takes one or more wifi_channel_spec as parameter
#define WIFI_TAG_WAKE_LOCK_EVENT 6 // takes a wake_lock_event struct as parameter
#define WIFI_TAG_ADDR1 7 // takes a 6 bytes MAC address as parameter
#define WIFI_TAG_ADDR2 8 // takes a 6 bytes MAC address as parameter
#define WIFI_TAG_ADDR3 9 // takes a 6 bytes MAC address as parameter
#define WIFI_TAG_ADDR4 10 // takes a 6 bytes MAC address as parameter
#define WIFI_TAG_TSF 11 // take a 64 bits TSF value as parameter
#define WIFI_TAG_IE 12  // take one or more specific 802.11 IEs parameter, IEs are in turn indicated
                        // in TLV format as per 802.11 spec
#define WIFI_TAG_INTERFACE 13 // take interface name as parameter
#define WIFI_TAG_REASON_CODE 14 // take a reason code as per 802.11 as parameter
#define WIFI_TAG_RATE_MBPS 15 // take a wifi rate in 0.5 mbps

typedef struct {
    u16 tag;
    u16 length; // length of value
    u8 value[0];
} __attribute__((packed)) tlv_log;

typedef struct {
    u16 event;
    tlv_log tlvs[0];    // separate parameter structure per event to be provided and optional data
                        // the event_data is expected to include an official android part, with some
                        // parameter as transmit rate, num retries, num scan result found etc...
                        // as well, event_data can include a vendor proprietary part which is
                        // understood by the developer only.
} __attribute__((packed)) wifi_ring_buffer_driver_connectivity_event;

// Ring buffer name for connectivity events ring
static char connectivity_event_ring_name[] = "wifi_connectivity_events";

// Ring buffer name for power events ring. note that power event are extremely frequents
// and thus should be stored in their own ring/file so as not to clobber connectivity events

typedef struct {
    int status;   // 0 taken, 1 released
    int reason;   // reason why this wake lock is taken
    char name[0]; // null terminated
} __attribute__((packed)) wake_lock_event;

typedef struct {
    u16 event;
    tlv_log tlvs[0];
} __attribute__((packed)) wifi_power_event;

static char power_event_ring_name[] = "wifi_power_events";

/**
 * This structure represent a logger entry within a ring buffer.
 * Wifi driver are responsible to manage the ring buffer and write the debug
 * information into those rings.
 *
 * In general, the debug entries can be used to store meaningful 802.11 information (SME, MLME,
 * connection and packet statistics) as well as vendor proprietary data that is specific to a
 * specific driver or chipset.
 * Binary entries can be used so as to store packet data or vendor specific information and
 * will be treated as blobs of data by android.
 *
 * A user land process will be started by framework so as to periodically retrieve the
 * data logged by drivers into their ring buffer, store the data into log files and include
 * the logs into android bugreports.
 */

#define RING_BUFFER_ENTRY_FLAGS_HAS_BINARY 1 // set for binary entries
#define RING_BUFFER_ENTRY_FLAGS_HAS_TIMESTAMP 2 // set if 64 bits timestamp is present

typedef struct {
    u16 entry_size;
    u8 flags;
    u8 type; // Per ring specific
    u64 timestamp; //present if has_timestamp bit is set.
    union {
        u8 data[0];
        wifi_ring_buffer_driver_connectivity_event connectivity_event;
        wifi_ring_per_packet_status_entry packet_status;
        wifi_power_event power_event;
        };
} __attribute__((packed)) wifi_ring_buffer_entry;

#define WIFI_RING_BUFFER_FLAG_HAS_BINARY_ENTRIES 0x00000001     // set if binary entries are present
#define WIFI_RING_BUFFER_FLAG_HAS_ASCII_ENTRIES  0x00000002     // set if ascii entries are present

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
   wifi_ring_buffer_id ring_id; // unique integer representing the ring
   u32 ring_buffer_byte_size;   // total memory size allocated for the buffer
   u32 verbose_level; //
   u32 written_bytes; // number of bytes that was written to the buffer by driver, monotonously increasing integer
   u32 read_bytes;  // number of bytes that was read from the buffer by user land, monotonously increasing integer
   u32 written_records;  // number of records that was written to the buffer by driver, monotonously increasing integer

} wifi_ring_buffer_status;

/**
 * Callback for reporting ring data
 *
 * The ring buffer data collection is event based:
 *    - driver calls on_ring_buffer_data when new records are available, the wifi_ring_buffer_status
 *      passed up to framework in the call back indicates to framework if more data is available in
 *      the ring buffer. It is not expected that driver will necessarily always empty the ring
 *      immediately as data is available, instead driver will report data every X seconds or if
 *      N bytes are available.
 *    - in the case where a bug report has to be captured, framework will require driver to upload
 *      all data immediately. This is indicated to driver when framework calls wifi_get_ringdata.
 *      When framework calls wifi_get_ring_data, driver will start sending all available data in the
 *      indicated ring by repeatedly invoking the on_ring_buffer_data callback
 *
 * The callback is called by driver whenever new data is
 */
typedef struct {
  void (*on_ring_buffer_data) (wifi_request_id id, wifi_ring_buffer_id ring_id, char *buffer, int buffer_size, wifi_ring_buffer_status *status);
} wifi_ring_buffer_data_handler;


/* api for framework to indicate driver has to upload and drain all data of a given ring */
wifi_error wifi_get_ring_data(wifi_request_id id,
        wifi_interface_handle iface, wifi_ring_buffer_id ring_id);


/**
 * API to trigger the debug collection.
 *  Unless his API is invoked - logging is not triggered.
 *  - verbose_level 0 corresponds to no collection
 *  - verbose_level 1 correspond to normal log level, with minimal user impact. this is the default value
 *  - verbose_level 2 are enabled when user is lazily trying to reproduce a problem, wifi performances and power
 *     can be impacted but device should not otherwise be significantly impacted
 *  - verbose_level 3+ are used when trying to actively debug a problem
 *
 * buffer_name represent the name of the ring for which data collection shall start.
 *
 * flags: TBD parameter used to enable/disable specific events on a ring
 * max_interval: maximum interval in seconds for driver to invoke on_ring_buffer_data, ignore if zero
 * min_data_size: minimum data size in buffer for driver to invoke on_ring_buffer_data, ignore if zero
 */

wifi_error wifi_start_logging(wifi_interface_handle iface, u32 verbose_level, u32 flags, u32 max_interval_sec, u32 min_data_size, u8 *buffer_name, wifi_ring_buffer_data_handler handler);

/* api to get the status of all ring buffers supported by driver */
wifi_error wifi_get_ring_buffers_status(wifi_request_id id,
        wifi_interface_handle iface, u32 num_buffers, wifi_ring_buffer_status *status);

/* api to collect a firmware memory dump for a given iface */
wifi_error wifi_get_firmware_memory_dump(wifi_request_id id,
        wifi_interface_handle iface, char ** buffer, int *buffer_size);

/* api to collect a firmware version string */
wifi_error wifi_get_firmware_version(wifi_request_id id,
        wifi_interface_handle iface, char *buffer, int buffer_size);

/* api to collect a driver version string */
wifi_error wifi_get_driver_version(wifi_request_id id,
        wifi_interface_handle iface, char *buffer, int buffer_size);


/* api to collect driver records */
wifi_error wifi_get_ringdata(wifi_request_id id,
        wifi_interface_handle iface, wifi_ring_buffer_id ring_id);


/* Feature set */
#define WIFI_LOGGER_MEMORY_DUMP_SUPPORTED 1
#define WIFI_LOGGER_PER_PACKET_TX_RX_STATUS_SUPPORTED 2

wifi_error wifi_get_logger_supported_feature_set(wifi_interface_handle handle, unsigned int *support);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /*__WIFI_HAL_STATS_ */

