/*
 * Copyright (C) 2016 The Android Open Source Project
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

#ifndef __NAN_H__
#define __NAN_H__

#include <net/if.h>
#include "wifi_hal.h"

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

/*****************************************************************************
 * Neighbour Aware Network Service Structures and Functions
 *****************************************************************************/

/*
  Definitions
  All multi-byte fields within all NAN protocol stack messages are assumed to be in Little Endian order.
*/

typedef int NanVersion;
typedef u16 transaction_id;
typedef u32 NanDataPathId;

#define NAN_MAC_ADDR_LEN                6
#define NAN_MAJOR_VERSION               2
#define NAN_MINOR_VERSION               0
#define NAN_MICRO_VERSION               0
#define NAN_MAX_SOCIAL_CHANNELS         3

/* NAN Maximum Lengths */
#define NAN_MAX_SERVICE_NAME_LEN                255
#define NAN_MAX_MATCH_FILTER_LEN                255
#define NAN_MAX_SERVICE_SPECIFIC_INFO_LEN       1024
#define NAN_MAX_VSA_DATA_LEN                    1024
#define NAN_MAX_MESH_DATA_LEN                   32
#define NAN_MAX_INFRA_DATA_LEN                  32
#define NAN_MAX_CLUSTER_ATTRIBUTE_LEN           255
#define NAN_MAX_SUBSCRIBE_MAX_ADDRESS           42
#define NAN_MAX_FAM_CHANNELS                    32
#define NAN_MAX_POSTDISCOVERY_LEN               5
#define NAN_MAX_FRAME_DATA_LEN                  504
#define NAN_DP_MAX_APP_INFO_LEN                 512

/*
  Definition of various NanResponseType
*/
typedef enum {
    NAN_RESPONSE_ENABLED                = 0,
    NAN_RESPONSE_DISABLED               = 1,
    NAN_RESPONSE_PUBLISH                = 2,
    NAN_RESPONSE_PUBLISH_CANCEL         = 3,
    NAN_RESPONSE_TRANSMIT_FOLLOWUP      = 4,
    NAN_RESPONSE_SUBSCRIBE              = 5,
    NAN_RESPONSE_SUBSCRIBE_CANCEL       = 6,
    NAN_RESPONSE_STATS                  = 7,
    NAN_RESPONSE_CONFIG                 = 8,
    NAN_RESPONSE_TCA                    = 9,
    NAN_RESPONSE_ERROR                  = 10,
    NAN_RESPONSE_BEACON_SDF_PAYLOAD     = 11,
    NAN_GET_CAPABILITIES                = 12,
    NAN_DP_INTERFACE_CREATE             = 13,
    NAN_DP_INTERFACE_DELETE             = 14,
    NAN_DP_INITIATOR_RESPONSE           = 15,
    NAN_DP_RESPONDER_RESPONSE           = 16,
    NAN_DP_SCHEDULE_UPDATE              = 17,
    NAN_DP_END                          = 18
} NanResponseType;

/* NAN Publish Types */
typedef enum {
    NAN_PUBLISH_TYPE_UNSOLICITED = 0,
    NAN_PUBLISH_TYPE_SOLICITED,
    NAN_PUBLISH_TYPE_UNSOLICITED_SOLICITED
} NanPublishType;

/* NAN Transmit Priorities */
typedef enum {
    NAN_TX_PRIORITY_NORMAL = 0,
    NAN_TX_PRIORITY_HIGH
} NanTxPriority;

/* NAN Statistics Request ID Codes */
typedef enum {
    NAN_STATS_ID_DE_PUBLISH = 0,
    NAN_STATS_ID_DE_SUBSCRIBE,
    NAN_STATS_ID_DE_MAC,
    NAN_STATS_ID_DE_TIMING_SYNC,
    NAN_STATS_ID_DE_DW,
    NAN_STATS_ID_DE
} NanStatsType;

/* NAN Protocol Event ID Codes */
typedef enum {
    NAN_EVENT_ID_DISC_MAC_ADDR = 0,
    NAN_EVENT_ID_STARTED_CLUSTER,
    NAN_EVENT_ID_JOINED_CLUSTER
} NanDiscEngEventType;

/* TCA Type */
typedef enum {
    NAN_TCA_ID_CLUSTER_SIZE = 0
} NanTcaType;

/*
  Various NAN Protocol Response code
*/
typedef enum {
    /* NAN Protocol Response Codes */
    NAN_STATUS_SUCCESS = 0,
    NAN_STATUS_TIMEOUT = 1,
    NAN_STATUS_DE_FAILURE = 2,
    NAN_STATUS_INVALID_MSG_VERSION = 3,
    NAN_STATUS_INVALID_MSG_LEN = 4,
    NAN_STATUS_INVALID_MSG_ID = 5,
    NAN_STATUS_INVALID_HANDLE = 6,
    NAN_STATUS_NO_SPACE_AVAILABLE = 7,
    NAN_STATUS_INVALID_PUBLISH_TYPE = 8,
    NAN_STATUS_INVALID_TX_TYPE = 9,
    NAN_STATUS_INVALID_MATCH_ALGORITHM = 10,
    NAN_STATUS_DISABLE_IN_PROGRESS = 11,
    NAN_STATUS_INVALID_TLV_LEN = 12,
    NAN_STATUS_INVALID_TLV_TYPE = 13,
    NAN_STATUS_MISSING_TLV_TYPE = 14,
    NAN_STATUS_INVALID_TOTAL_TLVS_LEN = 15,
    NAN_STATUS_INVALID_MATCH_HANDLE= 16,
    NAN_STATUS_INVALID_TLV_VALUE = 17,
    NAN_STATUS_INVALID_TX_PRIORITY = 18,
    NAN_STATUS_INVALID_CONNECTION_MAP = 19,
    NAN_STATUS_INVALID_TCA_ID = 20,
    NAN_STATUS_INVALID_STATS_ID = 21,
    NAN_STATUS_NAN_NOT_ALLOWED = 22,
    NAN_STATUS_NO_OTA_ACK = 23,
    NAN_STATUS_TX_FAIL = 24,
    /* 25-4095 Reserved */
    /* NAN Configuration Response codes */
    NAN_STATUS_INVALID_RSSI_CLOSE_VALUE = 4096,
    NAN_STATUS_INVALID_RSSI_MIDDLE_VALUE = 4097,
    NAN_STATUS_INVALID_HOP_COUNT_LIMIT = 4098,
    NAN_STATUS_INVALID_MASTER_PREFERENCE_VALUE = 4099,
    NAN_STATUS_INVALID_LOW_CLUSTER_ID_VALUE = 4100,
    NAN_STATUS_INVALID_HIGH_CLUSTER_ID_VALUE = 4101,
    NAN_STATUS_INVALID_BACKGROUND_SCAN_PERIOD = 4102,
    NAN_STATUS_INVALID_RSSI_PROXIMITY_VALUE = 4103,
    NAN_STATUS_INVALID_SCAN_CHANNEL = 4104,
    NAN_STATUS_INVALID_POST_NAN_CONNECTIVITY_CAPABILITIES_BITMAP = 4105,
    NAN_STATUS_INVALID_FURTHER_AVAILABILITY_MAP_NUMCHAN_VALUE = 4106,
    NAN_STATUS_INVALID_FURTHER_AVAILABILITY_MAP_DURATION_VALUE = 4107,
    NAN_STATUS_INVALID_FURTHER_AVAILABILITY_MAP_CLASS_VALUE = 4108,
    NAN_STATUS_INVALID_FURTHER_AVAILABILITY_MAP_CHANNEL_VALUE = 4109,
    NAN_STATUS_INVALID_FURTHER_AVAILABILITY_MAP_AVAILABILITY_INTERVAL_BITMAP_VALUE = 4110,
    NAN_STATUS_INVALID_FURTHER_AVAILABILITY_MAP_MAP_ID = 4111,
    NAN_STATUS_INVALID_POST_NAN_DISCOVERY_CONN_TYPE_VALUE = 4112,
    NAN_STATUS_INVALID_POST_NAN_DISCOVERY_DEVICE_ROLE_VALUE = 4113,
    NAN_STATUS_INVALID_POST_NAN_DISCOVERY_DURATION_VALUE = 4114,
    NAN_STATUS_INVALID_POST_NAN_DISCOVERY_BITMAP_VALUE = 4115,
    NAN_STATUS_MISSING_FUTHER_AVAILABILITY_MAP = 4116,
    NAN_STATUS_INVALID_BAND_CONFIG_FLAGS = 4117,
    NAN_STATUS_INVALID_RANDOM_FACTOR_UPDATE_TIME_VALUE = 4118,
    NAN_STATUS_INVALID_ONGOING_SCAN_PERIOD = 4119,
    NAN_STATUS_INVALID_DW_INTERVAL_VALUE = 4120,
    NAN_STATUS_INVALID_DB_INTERVAL_VALUE = 4121,
    /* 4122-8191 RESERVED */
    NAN_TERMINATED_REASON_INVALID = 8192,
    NAN_TERMINATED_REASON_TIMEOUT = 8193,
    NAN_TERMINATED_REASON_USER_REQUEST = 8194,
    NAN_TERMINATED_REASON_FAILURE = 8195,
    NAN_TERMINATED_REASON_COUNT_REACHED = 8196,
    NAN_TERMINATED_REASON_DE_SHUTDOWN = 8197,
    NAN_TERMINATED_REASON_DISABLE_IN_PROGRESS = 8198,
    NAN_TERMINATED_REASON_POST_DISC_ATTR_EXPIRED = 8199,
    NAN_TERMINATED_REASON_POST_DISC_LEN_EXCEEDED = 8200,
    NAN_TERMINATED_REASON_FURTHER_AVAIL_MAP_EMPTY = 8201
} NanStatusType;

/* NAN Transmit Types */
typedef enum {
    NAN_TX_TYPE_BROADCAST = 0,
    NAN_TX_TYPE_UNICAST
} NanTxType;

/* NAN Subscribe Type */
typedef enum {
    NAN_SUBSCRIBE_TYPE_PASSIVE = 0,
    NAN_SUBSCRIBE_TYPE_ACTIVE
} NanSubscribeType;

/* NAN Service Response Filter Attribute Bit */
typedef enum {
    NAN_SRF_ATTR_BLOOM_FILTER = 0,
    NAN_SRF_ATTR_PARTIAL_MAC_ADDR
} NanSRFType;

/* NAN Service Response Filter Include Bit */
typedef enum {
    NAN_SRF_INCLUDE_DO_NOT_RESPOND = 0,
    NAN_SRF_INCLUDE_RESPOND
} NanSRFIncludeType;

/* NAN Match indication type */
typedef enum {
    NAN_MATCH_ALG_MATCH_ONCE = 0,
    NAN_MATCH_ALG_MATCH_CONTINUOUS,
    NAN_MATCH_ALG_MATCH_NEVER
} NanMatchAlg;

/* NAN Transmit Window Type */
typedef enum {
    NAN_TRANSMIT_IN_DW = 0,
    NAN_TRANSMIT_IN_FAW
} NanTransmitWindowType;

/* NAN SRF State in Subscribe */
typedef enum {
    NAN_USE_SRF = 0,
    NAN_DO_NOT_USE_SRF
} NanSRFState;

/* NAN Include SSI in MatchInd */
typedef enum {
    NAN_SSI_NOT_REQUIRED_IN_MATCH_IND = 0,
    NAN_SSI_REQUIRED_IN_MATCH_IND
} NanSsiInMatchInd;

/* NAN DP security Configuration */
typedef enum {
    NAN_DP_CONFIG_NO_SECURITY = 0,
    NAN_DP_CONFIG_SECURITY
} NanDataPathSecurityCfgStatus;

/* Data request Responder's response */
typedef enum {
    NAN_DP_REQUEST_ACCEPT = 0,
    NAN_DP_REQUEST_REJECT
} NanDataPathResponseCode;

/* Schedule update status */
typedef enum {
    NAN_DP_SCHEDULE_UPDATE_SUCCESS = 0,
    NAN_DP_SCHEDULE_UPDATE_FAILURE
} NanDataPathScheduleUpdateStatus;

/* Nan/NDP Capabilites info */
typedef struct {
    u32 max_concurrent_nan_clusters;
    u32 max_publishes;
    u32 max_subscribes;
    u32 max_service_name_len;
    u32 max_match_filter_len;
    u32 max_total_match_filter_len;
    u32 max_service_specific_info_len;
    u32 max_vsa_data_len;
    u32 max_mesh_data_len;
    u32 max_ndi_interfaces;
    u32 max_ndp_sessions;
    u32 max_app_info_len;
} NanCapabilities;

/*
  Host can send Vendor specific attributes which the Discovery Engine can
  enclose in Beacons and/or Service Discovery frames transmitted.
  Below structure is used to populate that.
*/
typedef struct {
    /*
       0 = transmit only in the next discovery window
       1 = transmit in next 16 discovery window
    */
    u8 payload_transmit_flag;
    /*
       Below flags will determine in which all frames
       the vendor specific attributes should be included
    */
    u8 tx_in_discovery_beacon;
    u8 tx_in_sync_beacon;
    u8 tx_in_service_discovery;
    /* Organizationally Unique Identifier */
    u32 vendor_oui;
    /*
       vendor specific attribute to be transmitted
       vsa_len : Length of the vsa data.
     */
    u32 vsa_len;
    u8 vsa[NAN_MAX_VSA_DATA_LEN];
} NanTransmitVendorSpecificAttribute;


/*
  Discovery Engine will forward any Vendor Specific Attributes
  which it received as part of this structure.
*/
/* Mask to determine on which frames attribute was received */
#define RX_DISCOVERY_BEACON_MASK  0x01
#define RX_SYNC_BEACON_MASK       0x02
#define RX_SERVICE_DISCOVERY_MASK 0x04
typedef struct {
    /*
       Frames on which this vendor specific attribute
       was received. Mask defined above
    */
    u8 vsa_received_on;
    /* Organizationally Unique Identifier */
    u32 vendor_oui;
    /* vendor specific attribute */
    u32 attr_len;
    u8 vsa[NAN_MAX_VSA_DATA_LEN];
} NanReceiveVendorSpecificAttribute;

/*
   NAN Beacon SDF Payload Received structure
   Discovery engine sends the details of received Beacon or
   Service Discovery Frames as part of this structure.
*/
typedef struct {
    /* Frame data */
    u32 frame_len;
    u8 frame_data[NAN_MAX_FRAME_DATA_LEN];
} NanBeaconSdfPayloadReceive;

/*
  Host can set the Periodic scan parameters for each of the
  3(6, 44, 149) Social channels. Only these channels are allowed
  any other channels are rejected
*/
typedef enum {
    NAN_CHANNEL_24G_BAND = 0,
    NAN_CHANNEL_5G_BAND_LOW,
    NAN_CHANNEL_5G_BAND_HIGH
} NanChannelIndex;

/*
   Structure to set the Social Channel Scan parameters
   passed as part of NanEnableRequest/NanConfigRequest
*/
typedef struct {
    /*
       Dwell time of each social channel in milliseconds
       NanChannelIndex corresponds to the respective channel
       If time set to 0 then the FW default time will be used.
    */
    u8 dwell_time[NAN_MAX_SOCIAL_CHANNELS];

    /*
       Scan period of each social channel in seconds
       NanChannelIndex corresponds to the respective channel
       If time set to 0 then the FW default time will be used.
    */
    u16 scan_period[NAN_MAX_SOCIAL_CHANNELS];
} NanSocialChannelScanParams;

/*
  Host can send Post Connectivity Capability attributes
  to be included in Service Discovery frames transmitted
  as part of this structure.
*/
typedef struct {
    /*
       0 = transmit only in the next discovery window
       1 = transmit in next 16 discovery window
    */
    u8 payload_transmit_flag;
    /* 1 - Wifi Direct supported 0 - Not supported */
    u8 is_wfd_supported;
    /* 1 - Wifi Direct Services supported 0 - Not supported */
    u8 is_wfds_supported;
    /* 1 - TDLS supported 0 - Not supported */
    u8 is_tdls_supported;
    /* 1 - IBSS supported 0 - Not supported */
    u8 is_ibss_supported;
    /* 1 - Mesh supported 0 - Not supported */
    u8 is_mesh_supported;
    /*
       1 - NAN Device currently connect to WLAN Infra AP
       0 - otherwise
    */
    u8 wlan_infra_field;
} NanTransmitPostConnectivityCapability;

/*
  Discovery engine providing the post connectivity capability
  received.
*/
typedef struct {
    /* 1 - Wifi Direct supported 0 - Not supported */
    u8 is_wfd_supported;
    /* 1 - Wifi Direct Services supported 0 - Not supported */
    u8 is_wfds_supported;
    /* 1 - TDLS supported 0 - Not supported */
    u8 is_tdls_supported;
    /* 1 - IBSS supported 0 - Not supported */
    u8 is_ibss_supported;
    /* 1 - Mesh supported 0 - Not supported */
    u8 is_mesh_supported;
    /*
       1 - NAN Device currently connect to WLAN Infra AP
       0 - otherwise
    */
    u8 wlan_infra_field;
} NanReceivePostConnectivityCapability;

/*
  Indicates the availability interval duration associated with the
  Availability Intervals Bitmap field
*/
typedef enum {
    NAN_DURATION_16MS = 0,
    NAN_DURATION_32MS = 1,
    NAN_DURATION_64MS = 2
} NanAvailDuration;

/* Further availability per channel information */
typedef struct {
    /* Defined above */
    NanAvailDuration entry_control;
    /*
       1 byte field indicating the frequency band the NAN Device
       will be available as defined in IEEE Std. 802.11-2012
       Annex E Table E-4 Global Operating Classes
    */
    u8 class_val;
    /*
       1 byte field indicating the channel the NAN Device
       will be available.
    */
    u8 channel;
    /*
        Map Id - 4 bit field which identifies the Further
        availability map attribute.
    */
    u8 mapid;
    /*
       divides the time between the beginnings of consecutive Discovery
       Windows of a given NAN cluster into consecutive time intervals
       of equal durations. The time interval duration is specified by
       the Availability Interval Duration subfield of the Entry Control
       field.

       A Nan device that sets the i-th bit of the Availability
       Intervals Bitmap to 1 shall be present during the corresponding
       i-th time interval in the operation channel indicated by the
       Operating Class and Channel Number fields in the same Availability Entry.

       A Nan device that sets the i-th bit of the Availability Intervals Bitmap to
       0 may be present during the corresponding i-th time interval in the operation
       channel indicated by the Operating Class and Channel Number fields in the same
       Availability Entry.

       The size of the Bitmap is dependent upon the Availability Interval Duration
       chosen in the Entry Control Field.  The size can be either 1, 2 or 4 bytes long

       - Duration field is equal to 0, only AIB[0] is valid
       - Duration field is equal to 1, only AIB [0] and AIB [1] is valid
       - Duration field is equal to 2, AIB [0], AIB [1], AIB [2] and AIB [3] are valid
    */
    u32 avail_interval_bitmap;
} NanFurtherAvailabilityChannel;

/*
  Further availability map which can be sent and received from
  Discovery engine
*/
typedef struct {
    /*
       Number of channels indicates the number of channel
       entries which is part of fam
    */
    u8 numchans;
    NanFurtherAvailabilityChannel famchan[NAN_MAX_FAM_CHANNELS];
} NanFurtherAvailabilityMap;

/*
  Host can send Post-Nan Discovery attributes which the Discovery Engine can
  enclose in Service Discovery frames
*/
/* Possible connection types in Post NAN Discovery attributes */
typedef enum {
    NAN_CONN_WLAN_INFRA = 0,
    NAN_CONN_P2P_OPER = 1,
    NAN_CONN_WLAN_IBSS = 2,
    NAN_CONN_WLAN_MESH = 3,
    NAN_CONN_FURTHER_SERVICE_AVAILABILITY = 4,
    NAN_CONN_WLAN_RANGING = 5
} NanConnectionType;

/* Possible device roles in Post NAN Discovery attributes */
typedef enum {
    NAN_WLAN_INFRA_AP = 0,
    NAN_WLAN_INFRA_STA = 1,
    NAN_P2P_OPER_GO = 2,
    NAN_P2P_OPER_DEV = 3,
    NAN_P2P_OPER_CLI = 4
} NanDeviceRole;

/* Structure of Post NAN Discovery attribute */
typedef struct {
    /* Connection type of the host */
    NanConnectionType  type;
    /*
       Device role of the host based on
       the connection type
    */
    NanDeviceRole role;
    /*
       Flag to send the information as a single shot or repeated
       for next 16 discovery windows
       0 - Single_shot
       1 - next 16 discovery windows
    */
    u8 transmit_freq;
    /* Duration of the availability bitmask */
    NanAvailDuration duration;
    /* Availability interval bitmap based on duration */
    u32 avail_interval_bitmap;
    /*
       Mac address depending on the conn type and device role
       --------------------------------------------------
       | Conn Type  |  Device Role |  Mac address Usage  |
       --------------------------------------------------
       | WLAN_INFRA |  AP/STA      |   BSSID of the AP   |
       --------------------------------------------------
       | P2P_OPER   |  GO          |   GO's address      |
       --------------------------------------------------
       | P2P_OPER   |  P2P_DEVICE  |   Address of who    |
       |            |              |   would become GO   |
       --------------------------------------------------
       | WLAN_IBSS  |  NA          |   BSSID             |
       --------------------------------------------------
       | WLAN_MESH  |  NA          |   BSSID             |
       --------------------------------------------------
    */
    u8 addr[NAN_MAC_ADDR_LEN];
    /*
       Mandatory mesh id value if connection type is WLAN_MESH
       Mesh id contains 0-32 octet identifier and should be
       as per IEEE Std.802.11-2012 spec.
    */
    u16 mesh_id_len;
    u8 mesh_id[NAN_MAX_MESH_DATA_LEN];
    /*
       Optional infrastructure SSID if conn_type is set to
       NAN_CONN_WLAN_INFRA
    */
    u16 infrastructure_ssid_len;
    u8 infrastructure_ssid_val[NAN_MAX_INFRA_DATA_LEN];
} NanTransmitPostDiscovery;

/*
   Discovery engine providing the structure of Post NAN
   Discovery
*/
typedef struct {
    /* Connection type of the host */
    NanConnectionType  type;
    /*
       Device role of the host based on
       the connection type
    */
    NanDeviceRole role;
    /* Duration of the availability bitmask */
    NanAvailDuration duration;
    /* Availability interval bitmap based on duration */
    u32 avail_interval_bitmap;
    /*
       Map Id - 4 bit field which identifies the Further
       availability map attribute.
    */
    u8 mapid;
    /*
       Mac address depending on the conn type and device role
       --------------------------------------------------
       | Conn Type  |  Device Role |  Mac address Usage  |
       --------------------------------------------------
       | WLAN_INFRA |  AP/STA      |   BSSID of the AP   |
       --------------------------------------------------
       | P2P_OPER   |  GO          |   GO's address      |
       --------------------------------------------------
       | P2P_OPER   |  P2P_DEVICE  |   Address of who    |
       |            |              |   would become GO   |
       --------------------------------------------------
       | WLAN_IBSS  |  NA          |   BSSID             |
       --------------------------------------------------
       | WLAN_MESH  |  NA          |   BSSID             |
       --------------------------------------------------
    */
    u8 addr[NAN_MAC_ADDR_LEN];
    /*
       Mandatory mesh id value if connection type is WLAN_MESH
       Mesh id contains 0-32 octet identifier and should be
       as per IEEE Std.802.11-2012 spec.
    */
    u16 mesh_id_len;
    u8 mesh_id[NAN_MAX_MESH_DATA_LEN];
    /*
       Optional infrastructure SSID if conn_type is set to
       NAN_CONN_WLAN_INFRA
    */
    u16 infrastructure_ssid_len;
    u8 infrastructure_ssid_val[NAN_MAX_INFRA_DATA_LEN];
} NanReceivePostDiscovery;

/*
  Enable Request Message Structure
  The NanEnableReq message instructs the Discovery Engine to enter an operational state
*/
typedef struct {
    /* Mandatory parameters below */
    u8 master_pref;
    /*
      A cluster_low value matching cluster_high indicates a request to join
      a cluster with that value. If the requested cluster is not found the
      device will start its own cluster.
    */
    u16 cluster_low;
    u16 cluster_high;

    /*
      Optional configuration of Enable request.
      Each of the optional parameters have configure flag which
      determine whether configuration is to be passed or not.
    */
    u8 config_support_5g;
    u8 support_5g_val;
    /*
       BIT 0 is used to specify to include Service IDs in Sync/Discovery beacons
       0 - Do not include SIDs in any beacons
       1 - Include SIDs in all beacons.
       Rest 7 bits are count field which allows control over the number of SIDs
       included in the Beacon.  0 means to include as many SIDs that fit into
       the maximum allow Beacon frame size
    */
    u8 config_sid_beacon;
    u8 sid_beacon_val;
    /*
       The rssi values below should be specified without sign.
       For eg: -70dBm should be specified as 70.
    */
    u8 config_2dot4g_rssi_close;
    u8 rssi_close_2dot4g_val;

    u8 config_2dot4g_rssi_middle;
    u8 rssi_middle_2dot4g_val;

    u8 config_2dot4g_rssi_proximity;
    u8 rssi_proximity_2dot4g_val;

    u8 config_hop_count_limit;
    u8 hop_count_limit_val;

    /*
       Defines 2.4G channel access support
       0 - No Support
       1 - Supported
    */
    u8 config_2dot4g_support;
    u8 support_2dot4g_val;
    /*
       Defines 2.4G channels will be used for sync/discovery beacons
       0 - 2.4G channels not used for beacons
       1 - 2.4G channels used for beacons
    */
    u8 config_2dot4g_beacons;
    u8 beacon_2dot4g_val;
    /*
       Defines 2.4G channels will be used for Service Discovery frames
       0 - 2.4G channels not used for Service Discovery frames
       1 - 2.4G channels used for Service Discovery frames
    */
    u8 config_2dot4g_sdf;
    u8 sdf_2dot4g_val;
    /*
       Defines 5G channels will be used for sync/discovery beacons
       0 - 5G channels not used for beacons
       1 - 5G channels used for beacons
    */
    u8 config_5g_beacons;
    u8 beacon_5g_val;
    /*
       Defines 5G channels will be used for Service Discovery frames
       0 - 5G channels not used for Service Discovery frames
       1 - 5G channels used for Service Discovery frames
    */
    u8 config_5g_sdf;
    u8 sdf_5g_val;
    /*
       1 byte value which defines the RSSI in
       dBm for a close by Peer in 5 Ghz channels.
       The rssi values should be specified without sign.
       For eg: -70dBm should be specified as 70.
    */
    u8 config_5g_rssi_close;
    u8 rssi_close_5g_val;
    /*
       1 byte value which defines the RSSI value in
       dBm for a close by Peer in 5 Ghz channels.
       The rssi values should be specified without sign.
       For eg: -70dBm should be specified as 70.

    */
    u8 config_5g_rssi_middle;
    u8 rssi_middle_5g_val;
    /*
       1 byte value which defines the RSSI filter
       threshold.  Any Service Descriptors received above this
       value that are configured for RSSI filtering will be dropped.
       The rssi values should be specified without sign.
       For eg: -70dBm should be specified as 70.
    */
    u8 config_5g_rssi_close_proximity;
    u8 rssi_close_proximity_5g_val;
    /*
       1 byte quantity which defines the window size over
       which the “average RSSI” will be calculated over.
    */
    u8 config_rssi_window_size;
    u8 rssi_window_size_val;
    /*
       The 24 bit Organizationally Unique ID + the 8 bit Network Id.
    */
    u8 config_oui;
    u32 oui_val;
    /*
       NAN Interface Address, If not configured the Discovery Engine
       will generate a 6 byte Random MAC.
    */
    u8 config_intf_addr;
    u8 intf_addr_val[NAN_MAC_ADDR_LEN];
    /*
       If set to 1, the Discovery Engine will enclose the Cluster
       Attribute only sent in Beacons in a Vendor Specific Attribute
       and transmit in a Service Descriptor Frame.
    */
    u8 config_cluster_attribute_val;
    /*
       The periodicity in seconds between full scan’s to find any new
       clusters available in the area.  A Full scan should not be done
       more than every 10 seconds and should not be done less than every
       30 seconds.
    */
    u8 config_scan_params;
    NanSocialChannelScanParams scan_params_val;
    /*
       1 byte quantity which forces the Random Factor to a particular
       value for all transmitted Sync/Discovery beacons
    */
    u8 config_random_factor_force;
    u8 random_factor_force_val;
    /*
       1 byte quantity which forces the HC for all transmitted Sync and
       Discovery Beacon NO matter the real HC being received over the
       air.
    */
    u8 config_hop_count_force;
    u8 hop_count_force_val;

    /* channel frequency in MHz to enable Nan on */
    u8 config_24g_channel;
    wifi_channel channel_24g_val;

    u8 config_5g_channel;
    wifi_channel channel_5g_val;
} NanEnableRequest;

/*
  Publish Msg Structure
  Message is used to request the DE to publish the Service Name
  using the parameters passed into the Discovery Window
*/
typedef struct {
    u16 publish_id;/* id  0 means new publish, any other id is existing publish */
    u16 ttl; /* how many seconds to run for. 0 means forever until canceled */
    u16 period; /* periodicity of OTA unsolicited publish. Specified in increments of 500 ms */
    NanPublishType publish_type;/* 0= unsolicited, solicited = 1, 2= both */
    NanTxType tx_type; /* 0 = broadcast, 1= unicast  if solicited publish */
    u8 publish_count; /* number of OTA Publish, 0 means forever until canceled */
    u16 service_name_len; /* length of service name */
    u8 service_name[NAN_MAX_SERVICE_NAME_LEN];/* UTF-8 encoded string identifying the service */
    /*
       Field which specifies how the matching indication to host is controlled.
       0 - Match and Indicate Once
       1 - Match and Indicate continuous
       2 - Match and Indicate never. This means don't indicate the match to the host.
       3 - Reserved
    */
    NanMatchAlg publish_match_indicator;

    /*
       Sequence of values
       NAN Device that has invoked a Subscribe method corresponding to this Publish method
    */
    u16 service_specific_info_len;
    u8 service_specific_info[NAN_MAX_SERVICE_SPECIFIC_INFO_LEN];

    /*
       Ordered sequence of <length, value> pairs which specify further response conditions
       beyond the service name used to filter subscribe messages to respond to.
       This is only needed when the PT is set to NAN_SOLICITED or NAN_SOLICITED_UNSOLICITED.
    */
    u16 rx_match_filter_len;
    u8 rx_match_filter[NAN_MAX_MATCH_FILTER_LEN];

    /*
       Ordered sequence of <length, value> pairs to be included in the Discovery Frame.
       If present it is always sent in a Discovery Frame
    */
    u16 tx_match_filter_len;
    u8 tx_match_filter[NAN_MAX_MATCH_FILTER_LEN];

    /*
       flag which specifies that the Publish should use the configured RSSI
       threshold and the received RSSI in order to filter requests
       0 – ignore the configured RSSI threshold when running a Service
           Descriptor attribute or Service ID List Attribute through the DE matching logic.
       1 – use the configured RSSI threshold when running a Service
           Descriptor attribute or Service ID List Attribute through the DE matching logic.

    */
    u8 rssi_threshold_flag;

    /*
       8-bit bitmap which allows the Host to associate this publish
       with a particular Post-NAN Connectivity attribute
       which has been sent down in a NanConfigureRequest/NanEnableRequest
       message.  If the DE fails to find a configured Post-NAN
       connectivity attributes referenced by the bitmap,
       the DE will return an error code to the Host.
       If the Publish is configured to use a Post-NAN Connectivity
       attribute and the Host does not refresh the Post-NAN Connectivity
       attribute the Publish will be canceled and the Host will be sent
       a PublishTerminatedIndication message.
    */
    u8 connmap;
    /*
      Set/Enable corresponding bits to disable any indications that follow a publish.
      BIT0 - Disable publish termination indication.
      BIT1 - Disable match expired indication.
      BIT2 - Disable followUp indication received (OTA).
    */
    u8 recv_indication_cfg;
} NanPublishRequest;

/*
  Publish Cancel Msg Structure
  The PublishServiceCancelReq Message is used to request the DE to stop publishing
  the Service Name identified by the Publish Id in the message.
*/
typedef struct {
    u16 publish_id;
} NanPublishCancelRequest;

/*
  NAN Subscribe Structure
  The SubscribeServiceReq message is sent to the Discovery Engine
  whenever the Upper layers would like to listen for a Service Name
*/
typedef struct {
    u16 subscribe_id; /* id 0 means new subscribe, non zero is existing subscribe */
    u16 ttl; /* how many seconds to run for. 0 means forever until canceled */
    u16 period;/* periodicity of OTA Active Subscribe. Units in increments of 500 ms , 0 = attempt every DW*/

    /* Flag which specifies how the Subscribe request shall be processed. */
    NanSubscribeType subscribe_type; /* 0 - PASSIVE , 1- ACTIVE */

    /* Flag which specifies on Active Subscribes how the Service Response Filter attribute is populated.*/
    NanSRFType serviceResponseFilter; /* 0 - Bloom Filter, 1 - MAC Addr */

    /* Flag which specifies how the Service Response Filter Include bit is populated.*/
    NanSRFIncludeType serviceResponseInclude; /* 0=Do not respond if in the Address Set, 1= Respond */

    /* Flag which specifies if the Service Response Filter should be used when creating Subscribes.*/
    NanSRFState useServiceResponseFilter; /* 0=Do not send the Service Response Filter,1= send */

    /*
       Flag which specifies if the Service Specific Info is needed in
       the Publish message before creating the MatchIndication
    */
    NanSsiInMatchInd ssiRequiredForMatchIndication; /* 0=Not needed, 1= Required */

    /*
       Field which specifies how the matching indication to host is controlled.
       0 - Match and Indicate Once
       1 - Match and Indicate continuous
       2 - Match and Indicate never. This means don't indicate the match to the host.
       3 - Reserved
    */
    NanMatchAlg subscribe_match_indicator;

    /*
       The number of Subscribe Matches which should occur
       before the Subscribe request is automatically terminated.
    */
    u8 subscribe_count; /* If this value is 0 this field is not used by the DE.*/

    u16 service_name_len;/* length of service name */
    u8 service_name[NAN_MAX_SERVICE_NAME_LEN]; /* UTF-8 encoded string identifying the service */

    /* Sequence of values which further specify the published service beyond the service name*/
    u16 service_specific_info_len;
    u8 service_specific_info[NAN_MAX_SERVICE_SPECIFIC_INFO_LEN];

    /*
       Ordered sequence of <length, value> pairs used to filter out received publish discovery messages.
       This can be sent both for a Passive or an Active Subscribe
    */
    u16 rx_match_filter_len;
    u8 rx_match_filter[NAN_MAX_MATCH_FILTER_LEN];

    /*
       Ordered sequence of <length, value> pairs  included in the
       Discovery Frame when an Active Subscribe is used.
    */
    u16 tx_match_filter_len;
    u8 tx_match_filter[NAN_MAX_MATCH_FILTER_LEN];

    /*
       Flag which specifies that the Subscribe should use the configured RSSI
       threshold and the received RSSI in order to filter requests
       0 – ignore the configured RSSI threshold when running a Service
           Descriptor attribute or Service ID List Attribute through the DE matching logic.
       1 – use the configured RSSI threshold when running a Service
           Descriptor attribute or Service ID List Attribute through the DE matching logic.

    */
    u8 rssi_threshold_flag;

    /*
       8-bit bitmap which allows the Host to associate this Active
       Subscribe with a particular Post-NAN Connectivity attribute
       which has been sent down in a NanConfigureRequest/NanEnableRequest
       message.  If the DE fails to find a configured Post-NAN
       connectivity attributes referenced by the bitmap,
       the DE will return an error code to the Host.
       If the Subscribe is configured to use a Post-NAN Connectivity
       attribute and the Host does not refresh the Post-NAN Connectivity
       attribute the Subscribe will be canceled and the Host will be sent
       a SubscribeTerminatedIndication message.
    */
    u8 connmap;
    /*
       NAN Interface Address, conforming to the format as described in
       8.2.4.3.2 of IEEE Std. 802.11-2012.
    */
    u8 num_intf_addr_present;
    u8 intf_addr[NAN_MAX_SUBSCRIBE_MAX_ADDRESS][NAN_MAC_ADDR_LEN];
    /*
      Set/Enable corresponding bits to disable indications that follow a subscribe.
      BIT0 - Disable subscribe termination indication.
      BIT1 - Disable match expired indication.
      BIT2 - Disable followUp indication received (OTA).
    */
    u8 recv_indication_cfg;
} NanSubscribeRequest;

/*
  NAN Subscribe Cancel Structure
  The SubscribeCancelReq Message is used to request the DE to stop looking for the Service Name.
*/
typedef struct {
    u16 subscribe_id;
} NanSubscribeCancelRequest;

/*
  Transmit follow up Structure
  The TransmitFollowupReq message is sent to the DE to allow the sending of the Service_Specific_Info
  to a particular MAC address.
*/
typedef struct {
    /* Publish or Subscribe Id of an earlier Publish/Subscribe */
    u16 publish_subscribe_id;

    /*
       This Id is the Requestor Instance that is passed as
       part of earlier MatchInd/FollowupInd message.
    */
    u32 requestor_instance_id;
    u8 addr[NAN_MAC_ADDR_LEN]; /* Unicast address */
    NanTxPriority priority; /* priority of the request 2=high */
    NanTransmitWindowType dw_or_faw; /* 0= send in a DW, 1=send in FAW */

    /*
       Sequence of values which further specify the published service beyond
       the service name.
    */
    u16 service_specific_info_len;
    u8 service_specific_info[NAN_MAX_SERVICE_SPECIFIC_INFO_LEN];
    /*
      Set/Enable corresponding bits to disable responses after followUp.
      BIT0 - Disable followUp response from FW.
    */
    u8 recv_indication_cfg;
} NanTransmitFollowupRequest;

/*
  Stats Request structure
  The Discovery Engine can be queried at runtime by the Host processor for statistics
  concerning various parts of the Discovery Engine.
*/
typedef struct {
    NanStatsType stats_type; /* NAN Statistics Request Type */
    u8 clear; /* 0= Do not clear the stats and return the current contents , 1= Clear the associated stats  */
} NanStatsRequest;

/*
  Config Structure
  The NanConfigurationReq message is sent by the Host to the
  Discovery Engine in order to configure the Discovery Engine during runtime.
*/
typedef struct {
    u8 config_sid_beacon;
    u8 sid_beacon;
    u8 config_rssi_proximity;
    u8 rssi_proximity;
    u8 config_master_pref;
    u8 master_pref;
    /*
       1 byte value which defines the RSSI filter threshold.
       Any Service Descriptors received above this value
       that are configured for RSSI filtering will be dropped.
       The rssi values should be specified without sign.
       For eg: -70dBm should be specified as 70.
    */
    u8 config_5g_rssi_close_proximity;
    u8 rssi_close_proximity_5g_val;
    /*
      Optional configuration of Configure request.
      Each of the optional parameters have configure flag which
      determine whether configuration is to be passed or not.
    */
    /*
       2 byte quantity which defines the window size over
       which the “average RSSI” will be calculated over.
    */
    u8 config_rssi_window_size;
    u16 rssi_window_size_val;
    /*
       If set to 1, the Discovery Engine will enclose the Cluster
       Attribute only sent in Beacons in a Vendor Specific Attribute
       and transmit in a Service Descriptor Frame.
    */
    u8 config_cluster_attribute_val;
    /*
      The periodicity in seconds between full scan’s to find any new
      clusters available in the area.  A Full scan should not be done
      more than every 10 seconds and should not be done less than every
      30 seconds.
    */
    u8 config_scan_params;
    NanSocialChannelScanParams scan_params_val;
    /*
       1 byte quantity which forces the Random Factor to a particular
       value for all transmitted Sync/Discovery beacons
    */
    u8 config_random_factor_force;
    u8 random_factor_force_val;
    /*
       1 byte quantity which forces the HC for all transmitted Sync and
       Discovery Beacon NO matter the real HC being received over the
       air.
    */
    u8 config_hop_count_force;
    u8 hop_count_force_val;
    /* NAN Post Connectivity Capability */
    u8 config_conn_capability;
    NanTransmitPostConnectivityCapability conn_capability_val;
    /* NAN Post Discover Capability */
    u8 num_config_discovery_attr;
    NanTransmitPostDiscovery discovery_attr_val[NAN_MAX_POSTDISCOVERY_LEN];
    /* NAN Further availability Map */
    u8 config_fam;
    NanFurtherAvailabilityMap fam_val;
} NanConfigRequest;

/*
  TCA Structure
  The Discovery Engine can be configured to send up Events whenever a configured
  Threshold Crossing Alert (TCA) Type crosses an integral threshold in a particular direction.
*/
typedef struct {
    NanTcaType tca_type; /* Nan Protocol Threshold Crossing Alert (TCA) Codes */

    /* flag which control whether or not an event is generated for the Rising direction */
    u8 rising_direction_evt_flag; /* 0 - no event, 1 - event */

    /* flag which control whether or not an event is generated for the Falling direction */
    u8 falling_direction_evt_flag;/* 0 - no event, 1 - event */

    /* flag which requests a previous TCA request to be cleared from the DE */
    u8 clear;/*0= Do not clear the TCA, 1=Clear the TCA */

    /* 32 bit value which represents the threshold to be used.*/
    u32 threshold;
} NanTCARequest;

/*
  Beacon Sdf Payload Structure
  The Discovery Engine can be configured to publish vendor specific attributes as part of
  beacon or service discovery frame transmitted as part of this request..
*/
typedef struct {
    /*
       NanVendorAttribute will have the Vendor Specific Attribute which the
       vendor wants to publish as part of Discovery or Sync or Service discovery frame
    */
    NanTransmitVendorSpecificAttribute vsa;
} NanBeaconSdfPayloadRequest;

/* Publish statistics. */
typedef struct
{
    u32 validPublishServiceReqMsgs;
    u32 validPublishServiceRspMsgs;
    u32 validPublishServiceCancelReqMsgs;
    u32 validPublishServiceCancelRspMsgs;
    u32 validPublishRepliedIndMsgs;
    u32 validPublishTerminatedIndMsgs;
    u32 validActiveSubscribes;
    u32 validMatches;
    u32 validFollowups;
    u32 invalidPublishServiceReqMsgs;
    u32 invalidPublishServiceCancelReqMsgs;
    u32 invalidActiveSubscribes;
    u32 invalidMatches;
    u32 invalidFollowups;
    u32 publishCount;
    u32 publishNewMatchCount;
    u32 pubsubGlobalNewMatchCount;
} NanPublishStats;

/* Subscribe statistics. */
typedef struct
{
    u32 validSubscribeServiceReqMsgs;
    u32 validSubscribeServiceRspMsgs;
    u32 validSubscribeServiceCancelReqMsgs;
    u32 validSubscribeServiceCancelRspMsgs;
    u32 validSubscribeTerminatedIndMsgs;
    u32 validSubscribeMatchIndMsgs;
    u32 validSubscribeUnmatchIndMsgs;
    u32 validSolicitedPublishes;
    u32 validMatches;
    u32 validFollowups;
    u32 invalidSubscribeServiceReqMsgs;
    u32 invalidSubscribeServiceCancelReqMsgs;
    u32 invalidSubscribeFollowupReqMsgs;
    u32 invalidSolicitedPublishes;
    u32 invalidMatches;
    u32 invalidFollowups;
    u32 subscribeCount;
    u32 bloomFilterIndex;
    u32 subscribeNewMatchCount;
    u32 pubsubGlobalNewMatchCount;
} NanSubscribeStats;

/* NAN DW Statistics*/
typedef struct
{
    /* RX stats */
    u32 validFrames;
    u32 validActionFrames;
    u32 validBeaconFrames;
    u32 ignoredActionFrames;
    u32 ignoredBeaconFrames;
    u32 invalidFrames;
    u32 invalidActionFrames;
    u32 invalidBeaconFrames;
    u32 invalidMacHeaders;
    u32 invalidPafHeaders;
    u32 nonNanBeaconFrames;

    u32 earlyActionFrames;
    u32 inDwActionFrames;
    u32 lateActionFrames;

    /* TX stats */
    u32 framesQueued;
    u32 totalTRSpUpdates;
    u32 completeByTRSp;
    u32 completeByTp75DW;
    u32 completeByTendDW;
    u32 lateActionFramesTx;
} NanDWStats;

/* NAN MAC Statistics. */
typedef struct
{
    /* RX stats */
    u32 validFrames;
    u32 validActionFrames;
    u32 validBeaconFrames;
    u32 ignoredActionFrames;
    u32 ignoredBeaconFrames;
    u32 invalidFrames;
    u32 invalidActionFrames;
    u32 invalidBeaconFrames;
    u32 invalidMacHeaders;
    u32 invalidPafHeaders;
    u32 nonNanBeaconFrames;

    u32 earlyActionFrames;
    u32 inDwActionFrames;
    u32 lateActionFrames;

    /* TX stats */
    u32 framesQueued;
    u32 totalTRSpUpdates;
    u32 completeByTRSp;
    u32 completeByTp75DW;
    u32 completeByTendDW;
    u32 lateActionFramesTx;

    u32 twIncreases;
    u32 twDecreases;
    u32 twChanges;
    u32 twHighwater;
    u32 bloomFilterIndex;
} NanMacStats;

/* NAN Sync Statistics*/
typedef struct
{
    u64 currTsf;
    u64 myRank;
    u64 currAmRank;
    u64 lastAmRank;
    u32 currAmBTT;
    u32 lastAmBTT;
    u8  currAmHopCount;
    u8  currRole;
    u16 currClusterId;

    u64 timeSpentInCurrRole;
    u64 totalTimeSpentAsMaster;
    u64 totalTimeSpentAsNonMasterSync;
    u64 totalTimeSpentAsNonMasterNonSync;
    u32 transitionsToAnchorMaster;
    u32 transitionsToMaster;
    u32 transitionsToNonMasterSync;
    u32 transitionsToNonMasterNonSync;
    u32 amrUpdateCount;
    u32 amrUpdateRankChangedCount;
    u32 amrUpdateBTTChangedCount;
    u32 amrUpdateHcChangedCount;
    u32 amrUpdateNewDeviceCount;
    u32 amrExpireCount;
    u32 mergeCount;
    u32 beaconsAboveHcLimit;
    u32 beaconsBelowRssiThresh;
    u32 beaconsIgnoredNoSpace;
    u32 beaconsForOurCluster;
    u32 beaconsForOtherCluster;
    u32 beaconCancelRequests;
    u32 beaconCancelFailures;
    u32 beaconUpdateRequests;
    u32 beaconUpdateFailures;
    u32 syncBeaconTxAttempts;
    u32 syncBeaconTxFailures;
    u32 discBeaconTxAttempts;
    u32 discBeaconTxFailures;
    u32 amHopCountExpireCount;
} NanSyncStats;

/* NAN Misc DE Statistics */
typedef struct
{
    u32 validErrorRspMsgs;
    u32 validTransmitFollowupReqMsgs;
    u32 validTransmitFollowupRspMsgs;
    u32 validFollowupIndMsgs;
    u32 validConfigurationReqMsgs;
    u32 validConfigurationRspMsgs;
    u32 validStatsReqMsgs;
    u32 validStatsRspMsgs;
    u32 validEnableReqMsgs;
    u32 validEnableRspMsgs;
    u32 validDisableReqMsgs;
    u32 validDisableRspMsgs;
    u32 validDisableIndMsgs;
    u32 validEventIndMsgs;
    u32 validTcaReqMsgs;
    u32 validTcaRspMsgs;
    u32 validTcaIndMsgs;
    u32 invalidTransmitFollowupReqMsgs;
    u32 invalidConfigurationReqMsgs;
    u32 invalidStatsReqMsgs;
    u32 invalidEnableReqMsgs;
    u32 invalidDisableReqMsgs;
    u32 invalidTcaReqMsgs;
} NanDeStats;

/* Publish Response Message structure */
typedef struct {
    u16 publish_id;
} NanPublishResponse;

/* Subscribe Response Message structure */
typedef struct {
    u16 subscribe_id;
} NanSubscribeResponse;

/*
  Stats Response Message structure
  The Discovery Engine response to a request by the Host for statistics.
*/
typedef struct {
    NanStatsType stats_type;
    union {
        NanPublishStats publish_stats;
        NanSubscribeStats subscribe_stats;
        NanMacStats mac_stats;
        NanSyncStats sync_stats;
        NanDeStats de_stats;
        NanDWStats dw_stats;
    } data;
} NanStatsResponse;

/* Response returned for Initiators Data request */
typedef struct {
    /*
      Unique token Id generated on the initiator
      side used for a NDP session between two NAN devices
    */
    NanDataPathId ndp_instance_id;
} NanDataPathRequestResponse;

/*
  NAN Response messages
*/
typedef struct {
    NanStatusType status; /* contains the result code */
    u32 value; /* For error returns the value is returned which was in error */
    NanResponseType response_type; /* NanResponseType Definitions */
    union {
        NanPublishResponse publish_response;
        NanSubscribeResponse subscribe_response;
        NanStatsResponse stats_response;
        NanDataPathRequestResponse data_request_response;
        NanCapabilities nan_capabilities;
    } body;
} NanResponseMsg;

/*
  Publish Terminated
  The PublishTerminatedInd message is sent by the DE whenever a Publish
  terminates from a user-specified timeout or a unrecoverable error in the DE.
*/
typedef struct {
    /* Id returned during the initial Publish */
    u16 publish_id;
    NanStatusType reason;
} NanPublishTerminatedInd;

/*
  Match Indication
  The MatchInd message is sent once per responding MAC address whenever
  the Discovery Engine detects a match for a previous SubscribeServiceReq
  or PublishServiceReq.
*/
typedef struct {
    /* Publish or Subscribe Id of an earlier Publish/Subscribe */
    u16 publish_subscribe_id;
    /*
       A 32 bit Requestor Instance Id which is sent to the Application.
       This Id will be sent in any subsequent UnmatchInd/FollowupInd
       messages
    */
    u32 requestor_instance_id;
    u8 addr[NAN_MAC_ADDR_LEN];

    /*
       Sequence of octets which were received in a Discovery Frame matching the
       Subscribe Request.
    */
    u16 service_specific_info_len;
    u8 service_specific_info[NAN_MAX_SERVICE_SPECIFIC_INFO_LEN];

    /*
       Ordered sequence of <length, value> pairs received in the Discovery Frame
       matching the Subscribe Request.
    */
    u16 sdf_match_filter_len;
    u8 sdf_match_filter[NAN_MAX_MATCH_FILTER_LEN];

    /*
       flag to indicate if the Match occurred in a Beacon Frame or in a
       Service Discovery Frame.
         0 - Match occured in a Service Discovery Frame
         1 - Match occured in a Beacon Frame
    */
    u8 match_occured_flag;

    /*
       flag to indicate FW is out of resource and that it can no longer
       track this Service Name. The Host still need to send the received
       Match_Handle but duplicate MatchInd messages may be received on
       this Handle until the resource frees up.
         0 - FW is caching this match
         1 - FW is unable to cache this match
    */
    u8 out_of_resource_flag;

    /*
       If RSSI filtering was configured in NanSubscribeRequest then this
       field will contain the received RSSI value. 0 if not.
       All rssi values should be specified without sign.
       For eg: -70dBm should be specified as 70.
    */
    u8 rssi_value;

    /*
       optional attributes. Each optional attribute is associated with a flag
       which specifies whether the attribute is valid or not
    */
    /* NAN Post Connectivity Capability received */
    u8 is_conn_capability_valid;
    NanReceivePostConnectivityCapability conn_capability;

    /* NAN Post Discover Capability */
    u8 num_rx_discovery_attr;
    NanReceivePostDiscovery discovery_attr[NAN_MAX_POSTDISCOVERY_LEN];

    /* NAN Further availability Map */
    u8 num_chans;
    NanFurtherAvailabilityChannel famchan[NAN_MAX_FAM_CHANNELS];

    /* NAN Cluster Attribute */
    u8 cluster_attribute_len;
    u8 cluster_attribute[NAN_MAX_CLUSTER_ATTRIBUTE_LEN];
} NanMatchInd;

/*
  MatchExpired Indication
  The MatchExpiredInd message is sent whenever the Discovery Engine detects that
  a previously Matched Service has been gone for too long. If the previous
  MatchInd message for this Publish/Subscribe Id had the out_of_resource_flag
  set then this message will not be received
*/
typedef struct {
    /* Publish or Subscribe Id of an earlier Publish/Subscribe */
    u16 publish_subscribe_id;
    /*
       32 bit value sent by the DE in a previous
       MatchInd/FollowupInd to the application.
    */
    u32 requestor_instance_id;
} NanMatchExpiredInd;

/*
  Subscribe Terminated
  The SubscribeTerminatedInd message is sent by the DE whenever a
  Subscribe terminates from a user-specified timeout or a unrecoverable error in the DE.
*/
typedef struct {
    /* Id returned during initial Subscribe */
    u16 subscribe_id;
    NanStatusType reason;
} NanSubscribeTerminatedInd;

/*
  Followup Indication Message
  The FollowupInd message is sent by the DE to the Host whenever it receives a
  Followup message from another peer.
*/
typedef struct {
    /* Publish or Subscribe Id of an earlier Publish/Subscribe */
    u16 publish_subscribe_id;
    /*
       A 32 bit Requestor instance Id which is sent to the Application.
       This Id will be used in subsequent UnmatchInd/FollowupInd messages.
    */
    u32 requestor_instance_id;
    u8 addr[NAN_MAC_ADDR_LEN];

    /* Flag which the DE uses to decide if received in a DW or a FAW*/
    u8 dw_or_faw; /* 0=Received  in a DW, 1 = Received in a FAW*/

    /*
       Sequence of values which further specify the published service beyond
       the service name
    */
    u16 service_specific_info_len;
    u8 service_specific_info[NAN_MAX_SERVICE_SPECIFIC_INFO_LEN];
} NanFollowupInd;

/*
   Event data notifying the Mac address of the Discovery engine.
   which is reported as one of the Discovery engine event
*/
typedef struct {
    u8 addr[NAN_MAC_ADDR_LEN];
} NanMacAddressEvent;

/*
   Event data notifying the Cluster address of the cluster
   which is reported as one of the Discovery engine event
*/
typedef struct {
    u8 addr[NAN_MAC_ADDR_LEN];
} NanClusterEvent;

/*
  Discovery Engine Event Indication
  The Discovery Engine can inform the Host when significant events occur
  The data following the EventId is dependent upon the EventId type.
  In other words, each new event defined will carry a different
  structure of information back to the host.
*/
typedef struct {
    NanDiscEngEventType event_type; /* NAN Protocol Event Codes */
    union {
        /*
           MacAddressEvent which will have 6 byte mac address
           of the Discovery engine.
        */
        NanMacAddressEvent mac_addr;
        /*
           Cluster Event Data which will be obtained when the
           device starts a new cluster or joins a cluster.
           The event data will have 6 byte octet string of the
           cluster started or joined.
        */
        NanClusterEvent cluster;
    } data;
} NanDiscEngEventInd;

/* Cluster size TCA event*/
typedef struct {
    /* size of the cluster*/
    u32 cluster_size;
} NanTcaClusterEvent;

/*
  NAN TCA Indication
  The Discovery Engine can inform the Host when significant events occur.
  The data following the TcaId is dependent upon the TcaId type.
  In other words, each new event defined will carry a different structure
  of information back to the host.
*/
typedef struct {
    NanTcaType tca_type;
    /* flag which defines if the configured Threshold has risen above the threshold */
    u8 rising_direction_evt_flag; /* 0 - no event, 1 - event */

    /* flag which defines if the configured Threshold has fallen below the threshold */
    u8 falling_direction_evt_flag;/* 0 - no event, 1 - event */
    union {
        /*
           This event in obtained when the cluser size threshold
           is crossed. Event will have the cluster size
        */
        NanTcaClusterEvent cluster;
    } data;
} NanTCAInd;

/*
  NAN Disabled Indication
  The NanDisableInd message indicates to the upper layers that the Discovery
  Engine has flushed all state and has been shutdown.  When this message is received
  the DE is guaranteed to have left the NAN cluster it was part of and will have terminated
  any in progress Publishes or Subscribes.
*/
typedef struct {
    NanStatusType reason;
} NanDisabledInd;

/*
  NAN Beacon or SDF Payload Indication
  The NanBeaconSdfPayloadInd message indicates to the upper layers that information
  elements were received either in a Beacon or SDF which needs to be delivered
  outside of a Publish/Subscribe Handle.
*/
typedef struct {
    /* The MAC address of the peer which sent the attributes.*/
    u8 addr[NAN_MAC_ADDR_LEN];
    /*
       Optional attributes. Each optional attribute is associated with a flag
       which specifies whether the attribute is valid or not
    */
    /* NAN Receive Vendor Specific Attribute*/
    u8 is_vsa_received;
    NanReceiveVendorSpecificAttribute vsa;

    /* NAN Beacon or SDF Payload Received*/
    u8 is_beacon_sdf_payload_received;
    NanBeaconSdfPayloadReceive data;
} NanBeaconSdfPayloadInd;

/*
  Data request Initiator/Responder
  app/service related info
*/
typedef struct {
    u16 ndp_app_info_len;
    u8 ndp_app_info[NAN_DP_MAX_APP_INFO_LEN];
} NanDataPathAppInfo;

/* QoS configuration */
typedef enum {
    NAN_DP_CONFIG_NO_QOS = 0,
    NAN_DP_CONFIG_QOS
} NanDataPathQosCfg;

/* Configuration params of Data request Initiator/Responder */
typedef struct {
    /* Status Indicating Security/No Security */
    NanDataPathSecurityCfgStatus security_cfg;
    NanDataPathQosCfg qos_cfg;
} NanDataPathCfg;

/* Nan Data Path Initiator requesting a data session */
typedef struct {
    /*
     Unique Instance Id identifying the Responder's service.
     This is same as publish_id notified on the subscribe side
     in a publish/subscribe scenario
    */
    u16 service_instance_id; /* Value 0 for no publish/subscribe */
    /* Channel frequency in MHz to start data-path */
    wifi_channel channel;
    /*
      Discovery MAC addr of the publisher/peer
    */
    u8 peer_disc_mac_addr[NAN_MAC_ADDR_LEN];
    /*
     Interface name on which this NDP session is to be started.
     This will be the same interface name provided during interface
     create.
    */
    char ndp_iface[IFNAMSIZ+1];
    /* Initiator/Responder Security/QoS configuration */
    NanDataPathCfg ndp_cfg;
    /* App/Service information of the Initiator */
    NanDataPathAppInfo app_info;
} NanDataPathInitiatorRequest;

/*
  Data struct to initiate a data response on the responder side
  for an indication received with a data request
*/
typedef struct {
    /*
      Unique token Id generated on the initiator/responder
      side used for a NDP session between two NAN devices
    */
    NanDataPathId ndp_instance_id;
    /*
     Interface name on which this NDP session is to be started.
     This will be the same interface name provided during interface
     create.
    */
    char ndp_iface[IFNAMSIZ+1];
    /* Initiator/Responder Security/QoS configuration */
    NanDataPathCfg ndp_cfg;
    /* App/Service information of the responder */
    NanDataPathAppInfo app_info;
    /* Response Code indicating ACCEPT/REJECT/DEFER */
    NanDataPathResponseCode rsp_code;
} NanDataPathIndicationResponse;
/*
  Data struct used to update any Qos config of the
  existing NDP session. The update can be initiated
  on the initiator or the responder side.
*/
typedef struct {
    /*
      Unique token Id generated on the initiator/responder side
      used for a NDP session between two NAN devices
    */
    NanDataPathId ndp_instance_id;
    /* Initiator/Responder QoS configuration */
    NanDataPathQosCfg qos_cfg;
} NanDataPathScheduleUpdate;

/* NDP termination info */
typedef struct {
    u8 num_ndp_instances;
    /*
      Unique token Id generated on the initiator/responder side
      used for a NDP session between two NAN devices
    */
    NanDataPathId ndp_instance_id[];
} NanDataPathEndRequest;

/*
  Event indication received on the
  responder side when a Nan Data request or
  NDP session is initiated on the Initiator side
*/
typedef struct {
    /*
      Unique Instance Id corresponding to a service/session.
      This is similar to the publish_id generated on the
      publisher side
    */
    u16 service_instance_id;
    /* Discovery MAC addr of the peer/initiator */
    u8 peer_disc_mac_addr[NAN_MAC_ADDR_LEN];
    /*
      Unique token Id generated on the initiator/responder side
      used for a NDP session between two NAN devices
    */
    NanDataPathId ndp_instance_id;
    /* Initiator/Responder Security/QoS configuration */
    NanDataPathCfg ndp_cfg;
    /* App/Service information of the initiator */
    NanDataPathAppInfo app_info;
} NanDataPathRequestInd;

/*
 Event indication of data confirm is received on both
 initiator and responder side confirming a NDP session
*/
typedef struct {
    /*
      Unique token Id generated on the initiator/responder side
      used for a NDP session between two NAN devices
    */
    NanDataPathId ndp_instance_id;
    /*
      NDI mac address of the peer
      (required to derive target ipv6 address)
    */
    u8 peer_ndi_mac_addr[NAN_MAC_ADDR_LEN];
    /* Channel frequency in MHz on which data-path is started */
    wifi_channel channel;
    /* Initiator/Responder Security/QoS configuration */
    NanDataPathCfg ndp_cfg;
    /* App/Service information of Initiator/Responder */
    NanDataPathAppInfo app_info;
    /* Response code indicating ACCEPT/REJECT/DEFER */
    NanDataPathResponseCode rsp_code;
} NanDataPathConfirmInd;

/*
 Event indication received on the initiator
 side confirming a schedule.
*/
typedef struct {
    /*
      Unique token Id generated on the initiator/responder side
      used for a NDP session between two NAN devices
    */
    NanDataPathId ndp_instance_id;
    /* Discovery MAC addr of the peer */
    u8 peer_disc_mac_addr[NAN_MAC_ADDR_LEN];
    /* Initiator/Responder QoS configuration */
    NanDataPathQosCfg qos_cfg;
    /* Status indicating schedule update SUCCESS/FAILURE */
    NanDataPathScheduleUpdateStatus schedule_status;
} NanDataPathScheduleUpdateInd;

/*
  Event indication received on the
  initiator/responder side terminating
  a NDP session
*/
typedef struct {
    u8 num_ndp_instances;
    /*
      Unique token Id generated on the initiator/responder side
      used for a NDP session between two NAN devices
    */
    NanDataPathId ndp_instance_id[];
} NanDataPathEndInd;

/* Response and Event Callbacks */
typedef struct {
    /* NotifyResponse invoked to notify the status of the Request */
    void (*NotifyResponse)(transaction_id id, NanResponseMsg* rsp_data);
    /* Callbacks for various Events */
    void (*EventPublishTerminated)(NanPublishTerminatedInd* event);
    void (*EventMatch) (NanMatchInd* event);
    void (*EventMatchExpired) (NanMatchExpiredInd* event);
    void (*EventSubscribeTerminated) (NanSubscribeTerminatedInd* event);
    void (*EventFollowup) (NanFollowupInd* event);
    void (*EventDiscEngEvent) (NanDiscEngEventInd* event);
    void (*EventDisabled) (NanDisabledInd* event);
    void (*EventTca) (NanTCAInd* event);
    void (*EventBeaconSdfPayload) (NanBeaconSdfPayloadInd* event);
    void (*EventDataRequest)(NanDataPathRequestInd* event);
    void (*EventDataConfirm)(NanDataPathConfirmInd* event);
    void (*EventScheduleUpdate)(NanDataPathScheduleUpdateInd* event);
    void (*EventDataEnd)(NanDataPathEndInd* event);
} NanCallbackHandler;

/*  Enable NAN functionality. */
wifi_error nan_enable_request(transaction_id id,
                              wifi_interface_handle iface,
                              NanEnableRequest* msg);

/*  Disable NAN functionality. */
wifi_error nan_disable_request(transaction_id id,
                               wifi_interface_handle iface);

/*  Publish request to advertize a service. */
wifi_error nan_publish_request(transaction_id id,
                               wifi_interface_handle iface,
                               NanPublishRequest* msg);

/*  Cancel previous publish requests. */
wifi_error nan_publish_cancel_request(transaction_id id,
                                      wifi_interface_handle iface,
                                      NanPublishCancelRequest* msg);

/*  Subscribe request to search for a service. */
wifi_error nan_subscribe_request(transaction_id id,
                                 wifi_interface_handle iface,
                                 NanSubscribeRequest* msg);

/*  Cancel previous subscribe requests. */
wifi_error nan_subscribe_cancel_request(transaction_id id,
                                        wifi_interface_handle iface,
                                        NanSubscribeCancelRequest* msg);

/*  NAN transmit follow up request. */
wifi_error nan_transmit_followup_request(transaction_id id,
                                         wifi_interface_handle iface,
                                         NanTransmitFollowupRequest* msg);

/*  Request NAN statistics from Discovery Engine. */
wifi_error nan_stats_request(transaction_id id,
                             wifi_interface_handle iface,
                             NanStatsRequest* msg);

/* NAN configuration request. */
wifi_error nan_config_request(transaction_id id,
                              wifi_interface_handle iface,
                              NanConfigRequest* msg);

/* Configure the various Threshold crossing alerts. */
wifi_error nan_tca_request(transaction_id id,
                           wifi_interface_handle iface,
                           NanTCARequest* msg);

/*
    Set NAN Beacon or sdf payload to discovery engine.
    This instructs the Discovery Engine to begin publishing the
    received payload in any Beacon or Service Discovery Frame
    transmitted
*/
wifi_error nan_beacon_sdf_payload_request(transaction_id id,
                                         wifi_interface_handle iface,
                                         NanBeaconSdfPayloadRequest* msg);

/* Register NAN callbacks. */
wifi_error nan_register_handler(wifi_interface_handle iface,
                                NanCallbackHandler handlers);

/*  Get NAN HAL version. */
wifi_error nan_get_version(wifi_handle handle,
                           NanVersion* version);

/*  Get NAN capabilities. */
wifi_error nan_get_capabilities(transaction_id id,
                                wifi_interface_handle iface);

/* ========== Nan Data Path APIs ================ */
/* Create NAN Data Interface */
wifi_error nan_data_interface_create(transaction_id id,
                                     wifi_interface_handle iface,
                                     char* iface_name);

/* Delete NAN Data Interface */
wifi_error nan_data_interface_delete(transaction_id id,
                                     wifi_interface_handle iface,
                                     char* iface_name);

/* Initiate a NDP session: Initiator */
wifi_error nan_data_request_initiator(transaction_id id,
                                      wifi_interface_handle iface,
                                      NanDataPathInitiatorRequest* msg);

/*
 Response to a data indication received
 corresponding to a NDP session. An indication
 is received with a data request and the responder
 will send a data response
*/
wifi_error nan_data_indication_response(transaction_id id,
                                        wifi_interface_handle iface,
                                        NanDataPathIndicationResponse* msg);

/* NDL termination request: from either Initiator/Responder */
wifi_error nan_data_end(transaction_id id,
                        wifi_interface_handle iface,
                        NanDataPathEndRequest* msg);

/* NDL schedule update request per NDP session: from either Initiator/Responder */
wifi_error nan_data_schedule_update(transaction_id id,
                                    wifi_interface_handle iface,
                                    NanDataPathScheduleUpdate* msg);
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __NAN_H__ */
