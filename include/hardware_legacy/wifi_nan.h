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

#ifndef __NAN_H__
#define __NAN_H__

#include "wifi_hal.h"

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

/*****************************************************************************
 * NAN Discovery Service Structures and Functions
 *****************************************************************************/

/*
  Definitions
  All multi-byte fields within all NAN protocol stack messages are assumed to be in Little Endian order.
*/

typedef int NanVersion;

#define NAN_MAC_ADDR_LEN                6
#define NAN_COUNTRY_STRING_LEN          3
#define NAN_MAJOR_VERSION               1
#define NAN_MINOR_VERSION               0
#define NAN_MICRO_VERSION               0

/* NAN Maximum Lengths */
#define NAN_MAX_SERVICE_NAME_LEN                255
#define NAN_MAX_MATCH_FILTER_LEN                255
#define NAN_MAX_SERVICE_SPECIFIC_INFO_LEN       1024

/*
  Definition of various NanRequestType
*/
typedef enum {
    NAN_REQUEST_ENABLE                  =0,
    NAN_REQUEST_DISABLE                 =1,
    NAN_REQUEST_PUBLISH                 =2,
    NAN_REQUEST_PUBLISH_CANCEL          =3,
    NAN_REQUEST_TRANSMIT_FOLLOWUP       =4,
    NAN_REQUEST_SUBSCRIBE               =5,
    NAN_REQUEST_SUBSCRIBE_CANCEL        =6,
    NAN_REQUEST_STATS                   =7,
    NAN_REQUEST_CONFIG                  =8,
    NAN_REQUEST_TCA                     =9,
    NAN_REQUEST_LAST                    =0xFFFF
} NanRequestType;

/*
  Definition of various NanResponseType
*/
typedef enum {
    NAN_RESPONSE_ENABLED                =0,
    NAN_RESPONSE_DISABLED               =1,
    NAN_RESPONSE_PUBLISH                =2,
    NAN_RESPONSE_PUBLISH_CANCEL         =3,
    NAN_RESPONSE_TRANSMIT_FOLLOWUP      =4,
    NAN_RESPONSE_SUBSCRIBE              =5,
    NAN_RESPONSE_SUBSCRIBE_CANCEL       =6,
    NAN_RESPONSE_STATS                  =7,
    NAN_RESPONSE_CONFIG                 =8,
    NAN_RESPONSE_TCA                    =9,
    NAN_RESPONSE_ERROR                  =10,
    NAN_RESPONSE_UNKNOWN                =0xFFFF
} NanResponseType;

/*
  Definition of various NanIndication(events)
*/
typedef enum {
    NAN_INDICATION_PUBLISH_REPLIED         =0,
    NAN_INDICATION_PUBLISH_TERMINATED      =1,
    NAN_INDICATION_SUBSCRIBE_MATCH         =2,
    NAN_INDICATION_SUBSCRIBE_UNMATCH       =3,
    NAN_INDICATION_SUBSCRIBE_TERMINATED    =4,
    NAN_INDICATION_DE_EVENT                =5,
    NAN_INDICATION_FOLLOWUP                =6,
    NAN_INDICATION_DISABLED                =7,
    NAN_INDICATION_TCA                     =8,
    NAN_INDICATION_UNKNOWN                 =0xFFFF
} NanIndicationType;


/* NAN Publish Types */
typedef enum {
    NAN_PUBLISH_TYPE_UNSOLICITED = 0,
    NAN_PUBLISH_TYPE_SOLICITED,
    NAN_PUBLISH_TYPE_UNSOLICITED_SOLICITED,
    NAN_PUBLISH_TYPE_LAST,
} NanPublishType;

/* NAN Transmit Priorities */
typedef enum {
    NAN_TX_PRIORITY_LOW = 0,
    NAN_TX_PRIORITY_NORMAL,
    NAN_TX_PRIORITY_HIGH,
    NAN_TX_PRIORITY_LAST
} NanTxPriority;

/* NAN Statistics Request ID Codes */
typedef enum
{
    NAN_STATS_ID_FIRST = 0,
    NAN_STATS_ID_DE_PUBLISH = NAN_STATS_ID_FIRST,
    NAN_STATS_ID_DE_SUBSCRIBE,
    NAN_STATS_ID_DE_MAC,
    NAN_STATS_ID_DE_TIMING_SYNC,
    NAN_STATS_ID_DE_DW,
    NAN_STATS_ID_DE,
    NAN_STATS_ID_LAST
} NanStatsId;

/* NAN Protocol Event ID Codes */
typedef enum
{
    NAN_EVENT_ID_FIRST = 0,
    NAN_EVENT_ID_SELF_STA_MAC_ADDR = NAN_EVENT_ID_FIRST,
    NAN_EVENT_ID_STARTED_CLUSTER,
    NAN_EVENT_ID_JOINED_CLUSTER,
    NAN_EVENT_ID_LAST
} NanEventId;

/* TCA IDs */
typedef enum
{
    NAN_TCA_ID_FIRST = 0,
    NAN_TCA_ID_CLUSTER_SIZE = NAN_TCA_ID_FIRST,
    NAN_TCA_ID_LAST
} NanTcaId;

/*
  Various NAN Protocol Response code
*/
typedef enum
{
    /* NAN Protocol Response Codes */
    NAN_STATUS_SUCCESS = 0,
    NAN_STATUS_TIMEOUT,
    NAN_STATUS_DE_FAILURE,
    NAN_STATUS_INVALID_MSG_VERSION,
    NAN_STATUS_INVALID_MSG_LEN,
    NAN_STATUS_INVALID_MSG_ID,
    NAN_STATUS_INVALID_HANDLE,
    NAN_STATUS_NO_SPACE_AVAILABLE,
    NAN_STATUS_INVALID_PUBLISH_TYPE,
    NAN_STATUS_INVALID_TX_TYPE,
    NAN_STATUS_INVALID_MATCH_ALGORITHM,
    NAN_STATUS_DISABLE_IN_PROGRESS,
    NAN_STATUS_INVALID_TLV_LEN,
    NAN_STATUS_INVALID_TLV_TYPE,
    NAN_STATUS_MISSING_TLV_TYPE,
    NAN_STATUS_INVALID_TOTAL_TLVS_LEN,
    NAN_STATUS_INVALID_MATCH_HANDLE,
    NAN_STATUS_INVALID_TLV_VALUE,
    NAN_STATUS_INVALID_TX_PRIORITY,
    NAN_STATUS_INVALID_TCA_ID,
    NAN_STATUS_INVALID_STATS_ID,

    /* NAN Configuration Response codes */
    NAN_STATUS_INVALID_RSSI_CLOSE_VALUE = 128,
    NAN_STATUS_INVALID_RSSI_MEDIUM_VALUE,
    NAN_STATUS_INVALID_HOP_COUNT_LIMIT,
    NAN_STATUS_INVALID_CLUSTER_JOIN_COUNT,
    NAN_STATUS_INVALID_MIN_WAKE_DW_DURATION_VALUE,
    NAN_STATUS_INVALID_OFDM_DATA_RATE_VALUE,
    NAN_STATUS_INVALID_RANDOM_FACTOR_UPDATE_TIME_VALUE,
    NAN_STATUS_INVALID_MASTER_PREFERENCE_VALUE,
    NAN_STATUS_INVALID_EARLY_DW_WAKE_INTERVAL_VALUE,
    NAN_STATUS_INVALID_LOW_CLUSTER_ID_VALUE,
    NAN_STATUS_INVALID_HIGH_CLUSTER_ID_VALUE,
    NAN_STATUS_INVALID_INITIAL_SCAN_PERIOD,
    NAN_STATUS_INVALID_ONGOING_SCAN_PERIOD,
    NAN_STATUS_INVALID_RSSI_PROXIMITY_VALUE
} NanStatusType;

/*
  Various NAN Terminated Indication Code
*/
typedef enum
{
    NAN_TERMINATED_REASON_INVALID = 0,
    NAN_TERMINATED_REASON_TIMEOUT,
    NAN_TERMINATED_REASON_USER_REQUEST,
    NAN_TERMINATED_REASON_FAILURE,
    NAN_TERMINATED_REASON_COUNT_REACHED,
    NAN_TERMINATED_REASON_DE_SHUTDOWN,
    NAN_TERMINATED_REASON_DISABLE_IN_PROGRESS
} NanTerminatedStatus;

/* NAN Transmit Types */
typedef enum
{
    NAN_TX_TYPE_BROADCAST = 0,
    NAN_TX_TYPE_UNICAST,
    NAN_TX_TYPE_LAST
} NanTxType;

/* NAN Subscribe Type Bit */
#define NAN_SUBSCRIBE_TYPE_PASSIVE  0
#define NAN_SUBSCRIBE_TYPE_ACTIVE   1

/* NAN Service Response Filter Attribute Bit */
#define NAN_SRF_ATTR_BLOOM_FILTER       0
#define NAN_SRF_ATTR_PARTIAL_MAC_ADDR   1

/* NAN Service Response Filter Include Bit */
#define NAN_SRF_INCLUDE_DO_NOT_RESPOND  0
#define NAN_SRF_INCLUDE_RESPOND         1

/* NAN Match Algorithms */
typedef enum
{
    NAN_MATCH_ALG_FIRST = 0,
    NAN_MATCH_ALG_MATCH_ONCE = NAN_MATCH_ALG_FIRST,
    NAN_MATCH_ALG_MATCH_CONTINUOUS,
    NAN_MATCH_ALG_LAST
} NanMatchAlg;

/* NAN Header */
typedef struct {
    /*
    16-bit quantity which is allocated by the FW.
    Pass the Handle as 0xFFFF if the Host would like to set up a new
    Publish/Subscribe and the FW will pass back a valid handle in response msg.
    To update an already opened Publish/Subscribe Host can pass a Handle
    which has already been allocated by the FW.
    */
    u16 handle;

    /*
    16-bit quantity which is allocated in 2 contexts.  For all Request
    messages the TransactionId is allocated by the Service Layer and
    passed down to the DE.  In all Indication messages the TransactionId
    field is allocated by the DE.  There is no correlation between the
    TransactionIds allocated by the Service Layer and those allocated by the DE
    */
    u16 transaction_id;
} NanHeader;

/*
  Enable Request Message Structure
  The NanEnableReq message instructs the Discovery Engine to enter an operational state
*/
typedef struct {
    NanHeader header;
    u8 support_5g; /* default = 0 */
    u16 cluster_low; /* default = 0 */
    u16 cluster_high; /* default = 0 */
    u8 sid_beacon; /* default = 0x80 */
    u8 sync_disc_5g; /* default  = 1 i.e 5G Discovery frames only*/
    u8 rssi_close; /* default = 60 (-60 dBm) */
    u8 rssi_middle; /* default = 70 (-70 dBm) */
    u8 rssi_proximity; /* default = 70 (-70 dBm) */
    u8 hop_count_limit; /* default = 2 */
    u8 random_time; /* default  = 120 (DWs) */
    u8 master_pref; /* default = 0 */
    u8 periodic_scan_interval; /* default = 20 seconds */
    /* TBD: Google specific IE */
}NanEnableRequest;

/*
  Disable Request Message Structure
  The NanDisableReq message instructs the Discovery Engine to exit an operational state.
*/
typedef struct {
    NanHeader header;
}NanDisableRequest;

/*
  Publish Msg Structure
  Message is used to request the DE to publish the Service Name
  using the parameters passed into the Discovery Window
*/
typedef struct {
    NanHeader header;
    u16 ttl; /* how many seconds to run for. 0 means forever until canceled */
    u16 period; /* periodicity of OTA unsolicited publish. Specified in increments of 500 ms */
    u8 replied_event_flag; /* 1= RepliedEventInd needed, 0 = Not needed */
    NanPublishType publish_type;/* 0= unsolicited, solicited = 1, 2= both */
    NanTxType tx_type; /* 0 = broadcast, 1= unicast  if solicited publish */
    u8 publish_count; /* number of OTA Publish, 0 means forever until canceled */
    u16 service_name_len; /* length of service name */
    u8 service_name[NAN_MAX_SERVICE_NAME_LEN];/* UTF-8 encoded string identifying the service */

    /* Sequence of values which should be conveyed to the Discovery Engine of a
    NAN Device that has invoked a Subscribe method corresponding to this Publish method
    */
    u16 service_specific_info_len;
    u8 service_specific_info[NAN_MAX_SERVICE_SPECIFIC_INFO_LEN];

    /* Ordered sequence of <length, value> pairs which specify further response conditions
    beyond the service name used to filter subscribe messages to respond to.
    This is only needed when the PT is set to NAN_SOLICITED or NAN_SOLICITED_UNSOLICITED.
    */
    u16 rx_match_filter_len;
    u8 rx_match_filter[NAN_MAX_MATCH_FILTER_LEN];

    /* Ordered sequence of <length, value> pairs to be included in the Discovery Frame.
    If present it is always sent in a Discovery Frame
    */
    u16 tx_match_filter_len;
    u8 tx_match_filter[NAN_MAX_MATCH_FILTER_LEN];
}NanPublishRequest;

/*
  Publish Cancel Msg Structure
  The PublishServiceCancelReq Message is used to request the DE to stop publishing
  the Service Name identified by the handle in the message.
*/
typedef struct {
    NanHeader header;
}NanPublishCancelRequest;

/*
  NAN Subscribe Structure
  The SubscribeServiceReq message is sent to the Discovery Engine
  whenever the Upper layers would like to listen for a Service Name
*/
typedef struct {
    NanHeader header;
    u16 ttl; /* how many seconds to run for. 0 means forever until canceled */
    u16 period;/* periodicity of OTA Active Subscribe. Units in increments of 500 ms , 0 = attempt every DW*/

    /* Flag which specifies how the Subscribe request shall be processed. */
    u8 subscribe_type; /* 0 - PASSIVE , 1- ACTIVE */

    /* Flag which specifies on Active Subscribes how the Service Response Filter attribute is populated.*/
    u8 serviceResponseFilter; /* 0 - Bloom Filter, 1 - MAC Addr */

    /* Flag which specifies how the Service Response Filter Include bit is populated.*/
    u8 serviceResponseInclude; /* 0=Do not respond if in the Address Set, 1= Respond */

    /* Flag which specifies if the Service Response Filter should be used when creating Subscribes.*/
    u8 useServiceResponseFilter; /* 0=Do not send the Service Response Filter,1= send */

    /* Flag which specifies if the Service Specific Info is needed in the Publish message before creating the MatchIndication*/
    u8 ssiRequiredForMatchIndication; /* 0=Not needed, 1= Required */

    /* Field which allows the matching behavior to be controlled.  */
    NanMatchAlg subscribe_match; /* 0 - Match Once, 1 - Match continuous */

    /* The number of Subscribe Matches which should occur before the Subscribe request is automatically terminated.*/
    u8 subscribe_count; /* If this value is 0 this field is not used by the DE.*/

    u16 service_name_len;/* length of service name */
    u8 service_name[NAN_MAX_SERVICE_NAME_LEN]; /* UTF-8 encoded string identifying the service */

    /* Sequence of values which further specify the published service beyond the service name*/
    u16 service_specific_info_len;
    u8 service_specific_info[NAN_MAX_SERVICE_SPECIFIC_INFO_LEN];

    /* Ordered sequence of <length, value> pairs used to filter out received publish discovery messages.
    This can be sent both for a Passive or an Active Subscribe
    */
    u16 rx_match_filter_len;
    u8 rx_match_filter[NAN_MAX_MATCH_FILTER_LEN];

    /* Ordered sequence of <length, value> pairs  included in the Discovery Frame when an Active Subscribe is used.*/
    u16 tx_match_filter_len;
    u8 tx_match_filter[NAN_MAX_MATCH_FILTER_LEN];
}NanSubscribeRequest;


/*
  NAN Subscribe Cancel Structure
  The SubscribeCancelReq Message is used to request the DE to stop looking for the Service Name.
*/
typedef struct {
    NanHeader header;
}NanSubscribeCancelRequest;


/*
  Transmit follow up Structure
  The TransmitFollowupReq message is sent to the DE to allow the sending of the Service_Specific_Info
  to a particular MAC address.
*/
typedef struct {
    NanHeader header;
    u8 addr[NAN_MAC_ADDR_LEN]; /* Can be a broadcast/multicast or unicast address */
    NanTxPriority priority; /* priority of the request 0 = low, 1=normal, 2=high */
    u8 dw_or_faw; /* 0= send in a DW, 1=send in FAW */

    /* Sequence of values which further specify the published service beyond the service name
    Treated as service specific info in case dw_or_faw is set to 0
    Treated as extended service specific info in case dw_or_faw is set to non-zero*/
    u16 service_specific_info_len;
    u8 service_specific_info[NAN_MAX_SERVICE_SPECIFIC_INFO_LEN];
}NanTransmitFollowupRequest;

/*
  Stats Request structure
  The Discovery Engine can be queried at runtime by the Host processor for statistics
  concerning various parts of the Discovery Engine.
*/
typedef struct {
    NanHeader header;
    NanStatsId stats_id; /* NAN Statistics Request ID Codes */
    u8 clear; /* 0= Do not clear the stats and return the current contents , 1= Clear the associated stats  */
}NanStatsRequest;

/*
  Config Structure
  The NanConfigurationReq message is sent by the Host to the
  Discovery Engine in order to configure the Discovery Engine during runtime.
*/
typedef struct {
    NanHeader header;
    u8 sid_beacon; /* default = 0x80 */
    u8 sync_disc_5g; /* default  = 1 i.e 5G Discovery frames only*/
    u8 rssi_proximity; /* default = 70 (-70 dBm) */
    u8 random_time; /* default  = 120 (DWs) */
    u8 master_pref; /* default = 0 */
    u8 periodic_scan_interval; /* default = 20 seconds */
    /* The number of Additional Discovery Window slots in
       increments of 16 ms.  Since each DW is 512 TUs apart
       and the DW takes up 1 slot, the maximum number of additional
       slots which can be specified is 31.  This is a hint to the
       scheduler and there is no guarantee that all 31 slots will
       be available because of MCC and BT Coexistence channel usage
    */
    u8 additional_disc_window_slots; /* default = 0.*/
}NanConfigRequest;

/*
  TCA Structure
  The Discovery Engine can be configured to send up Events whenever a configured
  Threshold Crossing Alert (TCA) Id crosses an integral threshold in a particular direction.
*/
typedef struct {
    NanHeader header;
    NanTcaId tca_id; /* Nan Protocol Threshold Crossing Alert (TCA) Codes */

    /* flag which control whether or not an event is generated for the Rising direction */
    u8 rising_direction_evt_flag; /* 0 - no event, 1 - event */

    /* flag which control whether or not an event is generated for the Falling direction */
    u8 falling_direction_evt_flag;/* 0 - no event, 1 - event */

    /* flag which requests a previous TCA request to be cleared from the DE */
    u8 clear;/*0= Do not clear the TCA, 1=Clear the TCA */

    /* 32 bit value which represents the threshold to be used.*/
    u32 threshold;
}NanTCARequest;

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
} NanSubscribeStats;

/* NAN MAC Statistics. Used for MAC and DW statistics. */
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

    /* Misc stats - ignored for DW. */
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
    u32 discBeaconTxAttempts;
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

/*
  Stats Response Message structure
  The Discovery Engine response to a request by the Host for statistics.
*/
typedef struct {
    NanStatsId stats_id;
    union {
        NanPublishStats publish_stats;
        NanSubscribeStats subscribe_stats;
        NanMacStats mac_stats;
        NanSyncStats sync_stats;
        NanDeStats de_stats;
    }data;
}NanStatsResponse;

/*
  NAN Response messages
*/
typedef struct {
    NanHeader header;
    u16 status; /* contains the result code */
    u16 value; /* For error returns the value is returned which was in error */
    NanResponseType response_type; /* NanResponseType Definitions */
    union {
        NanStatsResponse stats_response;
    }body;
}NanResponseMsg;


/*
  Publish Replied Indication
  The PublishRepliedInd Message is sent by the DE when an Active Subscribe is
  received over the air and it matches a Solicited PublishServiceReq which had
  been created with the replied_event_flag set.
*/
typedef struct {
    NanHeader header;
    u8 addr[NAN_MAC_ADDR_LEN];
}NanPublishRepliedInd;

/*
  Publish Terminated
  The PublishTerminatedInd message is sent by the DE whenever a Publish
  terminates from a user-specified timeout or a unrecoverable error in the DE.
*/
typedef struct {
    NanHeader header;
    NanTerminatedStatus reason;
}NanPublishTerminatedInd;

/*
  Subscribe Match Indication
  The SubscribeMatchInd message is sent once per responding MAC address whenever
  the Discovery Engine detects a match for a previous SubscribeServiceReq.
*/
typedef struct {
    NanHeader header;

    /* a 16 bit Handle which is sent to the Application.  This handle will be sent in any subsequent
    UnmatchInd messages rather than resending the Match_Filter/Service_Specific_Info TLVs
    The Match_Handle is a DE resource and it is of limited quantity.  In the event that the DE
    runs out of Match_Handles the DE will still send the SubscribeMatchInd message but will
    set the Match_Handle to MATCH_HANDLE_MATCH_POOL_EXHAUSTED=0xFFFF
    */
    u16 match_handle;
    u8 addr[NAN_MAC_ADDR_LEN];

    /* Sequence of octets which were received in a Discovery Frame matching this
    Subscribe Request.*/
    u16 service_specific_info_len;
    u8 service_specific_info[NAN_MAX_SERVICE_NAME_LEN];

    /* Ordered sequence of <length, value> pairs received in the Discovery Frame
    matching this Subscribe Request.*/
    u16 sdf_match_filter_len;
    u8 sdf_match_filter[NAN_MAX_MATCH_FILTER_LEN];
}NanSubscribeMatchInd;

/*
  Subscribe UnMatch
  The SubscribeUnmatchInd message is sent whenever the Discovery Engine detects that
  a previously Matched Subscribed Service Name has been gone for too long.
  If the previous SubscribeMatchInd message contained a Match_Handle equal to
  MATCH_HANDLE_MATCH_POOL_EXHAUSTED then this message will not be sent to the Host.
*/
typedef struct {
    NanHeader header;
    /* 16 bit value sent by the DE in a previous SubscribeMatchInd to the application. */
    u16 match_handle;
}NanSubscribeUnmatchInd;

/*
  Subscribe Terminated
  The SubscribeTerminatedInd message is sent by the DE whenever a
  Subscribe terminates from a user-specified timeout or a unrecoverable error in the DE.
*/
typedef struct {
    NanHeader header;
    NanTerminatedStatus reason;
}NanSubscribeTerminatedInd;

/*
  Followup Indication Message
  The FollowupInd message is sent by the DE to the Host whenever it receives a
  Followup message from another peer.
*/
typedef struct {
    NanHeader header;
    u8 addr[NAN_MAC_ADDR_LEN];

    /* Flag which the DE uses to decide if received in a DW or a FAW*/
    u8 dw_or_faw; /* 0=Received  in a DW, 1 = Received in a FAW*/

    /* Sequence of values which further specify the published service beyond the service name
    Service specific info in case dw_or_faw is set to 0
    Extended service specific info in case dw_or_faw is set to non-zero*/
    u16 service_specific_info_len;
    u8 service_specific_info[NAN_MAX_SERVICE_SPECIFIC_INFO_LEN];
}NanFollowupInd;

/* Selfstaevent data*/
typedef struct {
    u8 addr[NAN_MAC_ADDR_LEN];
}NanSelfStaEvent;

/* joined or Started cluster data*/
typedef struct {
    u8 addr[NAN_MAC_ADDR_LEN];
}NanClusterEventData;

/*
  Discovery Engine Event Indication
  The Discovery Engine can inform the Host when significant events occur
  The data following the EventId is dependent upon the EventId type.
  In other words, each new event defined will carry a different
  structure of information back to the host.
*/
typedef struct {
    NanHeader header;
    NanEventId event_id; /* NAN Protocol Event Codes */
    union {
        /* SelfStaEvent which will have 6 byte mac address
           of the Discovery engine.*/
        NanSelfStaEvent self_sta;
        /* Cluster Event Data which will be obtained when the
           device starts a new cluster or joins a cluster.
           The event data will have 6 byte octet string of the
           cluster started or joined.*/
        NanClusterEventData cluster;
    }data;
}NanDiscEngEventInd;

/* Cluster size TCA data*/
typedef struct {
    u16 cluster_size;
}NanTcaClusterData;

/*
  NAN TCA Indication
  The Discovery Engine can inform the Host when significant events occur.
  The data following the TcaId is dependent upon the TcaId type.
  In other words, each new event defined will carry a different structure
  of information back to the host.
*/
typedef struct {
    NanHeader header;
    NanTcaId tca_id;
    union {
        /* This event in obtained when the cluser size threshold
           is crossed. Event will have the cluster size */
        NanTcaClusterData cluster;
    }data;
}NanTCAInd;

/*
  NAN Disabled Indication
  The NanDisableInd message indicates to the upper layers that the Discovery
  Engine has flushed all state and has been shutdown.  When this message is received
  the DE is guaranteed to have left the NAN cluster it was part of and will have terminated
  any in progress Publishes or Subscribes.
*/
typedef struct {
    NanHeader header;
    NanStatusType reason;
}NanDisabledInd;

/* Response and Event Callbacks */
typedef struct {
    /* NotifyResponse invoked to notify the status of the Request */
    void (*NotifyResponse)(NanResponseMsg* rsp_data);
    /* Various Events Callback */
    void (*EventPublishReplied)(NanPublishRepliedInd* event);
    void (*EventPublishTerminated)(NanPublishTerminatedInd* event);
    void (*EventSubscribeMatch) (NanSubscribeMatchInd* event);
    void (*EventSubscribeUnMatch) (NanSubscribeUnmatchInd* event);
    void (*EventSubscribeTerminated) (NanSubscribeTerminatedInd* event);
    void (*EventFollowup) (NanFollowupInd* event);
    void (*EventDiscEngEvent) (NanDiscEngEventInd* event);
    void (*EventDisabled) (NanDisabledInd* event);
    void (*EventTca) (NanTCAInd* event);
} NanCallbackHandler;


/*  Function to send NAN request to the wifi driver.*/
wifi_error nan_enable_request(wifi_request_id id,
                              wifi_handle handle,
                              NanEnableRequest* msg);

/*  Function to send NAN request to the wifi driver.*/
wifi_error nan_disable_request(wifi_request_id id,
                               wifi_handle handle,
                               NanDisableRequest* msg);

/*  Function to send NAN request to the wifi driver.*/
wifi_error nan_publish_request(wifi_request_id id,
                               wifi_handle handle,
                               NanPublishRequest* msg);

/*  Function to send NAN request to the wifi driver.*/
wifi_error nan_publish_cancel_request(wifi_request_id id,
                                      wifi_handle handle,
                                      NanPublishCancelRequest* msg);

/*  Function to send NAN request to the wifi driver.*/
wifi_error nan_subscribe_request(wifi_request_id id,
                                 wifi_handle handle,
                                 NanSubscribeRequest* msg);

/*  Function to send NAN request to the wifi driver.*/
wifi_error nan_subscribe_cancel_request(wifi_request_id id,
                                        wifi_handle handle,
                                        NanSubscribeCancelRequest* msg);

/*  Function to send NAN request to the wifi driver.*/
wifi_error nan_transmit_followup_request(wifi_request_id id,
                                         wifi_handle handle,
                                         NanTransmitFollowupRequest* msg);

/*  Function to send NAN statistics request to the wifi driver.*/
wifi_error nan_stats_request(wifi_request_id id,
                             wifi_handle handle,
                             NanStatsRequest* msg);

/*  Function to send NAN configuration request to the wifi driver.*/
wifi_error nan_config_request(wifi_request_id id,
                              wifi_handle handle,
                              NanConfigRequest* msg);

/*  Function to send NAN request to the wifi driver.*/
wifi_error nan_tca_request(wifi_request_id id,
                           wifi_handle handle,
                           NanTCARequest* msg);

/*  Function to register NAN callback */
wifi_error nan_register_handler(wifi_handle handle,
                                NanCallbackHandler handlers);

/*  Function to get version of the NAN HAL */
wifi_error nan_get_version(wifi_handle handle,
                           NanVersion* version);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __NAN_H__ */
