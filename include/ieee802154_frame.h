#ifndef IEEE802154_FRAME_H
#define IEEE802154_FRAME_H

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include "esp_assert.h"

// IEEE 802.15.4 FCF field value enumerations
typedef enum {
    IEEE802154_FRAME_TYPE_BEACON    = 0x0, // Beacon frame
    IEEE802154_FRAME_TYPE_DATA      = 0x1, // Data frame
    IEEE802154_FRAME_TYPE_ACK       = 0x2, // Acknowledgment frame
    IEEE802154_FRAME_TYPE_MAC_CMD   = 0x3, // MAC Command frame
    // 0x4 to 0x7 are reserved
} ieee802154_frame_type_t;

typedef enum {
    IEEE802154_ADDR_MODE_NONE       = 0x0, // No address
    IEEE802154_ADDR_MODE_RESERVED   = 0x1, // Reserved
    IEEE802154_ADDR_MODE_SHORT      = 0x2, // 16-bit short address
    IEEE802154_ADDR_MODE_EXTENDED   = 0x3, // 64-bit extended address
} ieee802154_addr_mode_t;

typedef enum {
    IEEE802154_VERSION_2003         = 0x0, // IEEE 802.15.4-2003
    IEEE802154_VERSION_2006         = 0x1, // IEEE 802.15.4-2006
    IEEE802154_VERSION_RESERVED1    = 0x2, // Reserved
    IEEE802154_VERSION_RESERVED2    = 0x3, // Reserved
} ieee802154_version_t;

// IEEE 802.15.4 Frame Control Field (FCF) structure
// Bit fields are ordered LSB to MSB to match IEEE 802.15.4 specification
typedef struct {
    uint8_t frameType                  : 3; // Frame Type (bits 0-2)
    uint8_t securityEnabled            : 1; // Security Enabled (bit 3)
    uint8_t framePending               : 1; // Frame Pending (bit 4)
    uint8_t ackRequest                 : 1; // Acknowledgment Request (bit 5)
    uint8_t panIdCompression           : 1; // PAN ID Compression (bit 6)
    uint8_t reserved                   : 1; // Reserved (bit 7)
    uint8_t sequenceNumberSuppression  : 1; // Sequence Number Suppression (bit 8)
    uint8_t informationElementsPresent : 1; // Information Elements Present (bit 9)
    uint8_t destAddrMode               : 2; // Destination Address Mode (bits 10-11)
    uint8_t frameVersion               : 2; // Frame Version (bits 12-13)
    uint8_t srcAddrMode                : 2; // Source Address Mode (bits 14-15)
} ieee802154_fcf_t;

// IEEE 802.15.4 MAC frame structure
typedef struct {
    ieee802154_fcf_t fcf;               // Frame Control Field
    uint8_t sequenceNumber;             // Sequence Number (if not suppressed)
    uint16_t destPanId;                 // Destination PAN ID (if present)
    uint8_t destAddress[8];             // Destination Address (0, 2, or 8 bytes)
    uint16_t srcPanId;                  // Source PAN ID (if present)
    uint8_t srcAddress[8];              // Source Address (0, 2, or 8 bytes)
    uint8_t destAddrLen;                // Length of destination address
    uint8_t srcAddrLen;                 // Length of source address
    size_t payloadLen;                  // Length of payload
    uint8_t *payload;                   // Pointer to payload data
    uint8_t rssi_lqi;                   // RSSI and LQI (combined in 1 byte)
} ieee802154_frame_t;

// Constants
#define IEEE802154_FCF_SIZE 2
#define IEEE802154_MAX_ADDR_LEN 8
#define IEEE802154_PAN_ID_LEN 2
#define IEEE802154_RSSI_LQI_SIZE 1 // 1 byte for combined RSSI and LQI

// Ensure FCF structure is exactly 2 bytes
ESP_STATIC_ASSERT(sizeof(ieee802154_fcf_t) == IEEE802154_FCF_SIZE, "ieee802154_fcf_t must be 2 bytes");

// Public API
bool ieee802154_frame_parse(const uint8_t *data, size_t len, ieee802154_frame_t *frame, bool verbose);
size_t ieee802154_frame_build(const ieee802154_frame_t *frame, uint8_t *buffer, size_t buffer_size);
const char* ieee802154_frame_type_to_str(uint8_t frameType);

#endif // IEEE802154_FRAME_H