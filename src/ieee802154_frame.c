#include <esp_log.h>
#include <string.h>
#include "ieee802154_frame.h"

static const char *TAG = "IEEE802154";

// Internal: Process FCF for debugging
static void process_fcf(const ieee802154_fcf_t *fcf, bool verbose) {
    if (!verbose) return;

    // Check Frame Type
    switch (fcf->frameType) {
        case IEEE802154_FRAME_TYPE_BEACON:
            ESP_LOGI(TAG, "Frame type: Beacon");
            break;
        case IEEE802154_FRAME_TYPE_DATA:
            ESP_LOGI(TAG, "Frame type: Data");
            break;
        case IEEE802154_FRAME_TYPE_ACK:
            ESP_LOGI(TAG, "Frame type: ACK");
            break;
        case IEEE802154_FRAME_TYPE_MAC_CMD:
            ESP_LOGI(TAG, "Frame type: MAC Command");
            break;
        default:
            ESP_LOGW(TAG, "Frame type: Reserved (0x%x)", fcf->frameType);
            break;
    }

    // Check Destination Addressing Mode
    switch (fcf->destAddrMode) {
        case IEEE802154_ADDR_MODE_NONE:
            ESP_LOGI(TAG, "Destination address: None");
            break;
        case IEEE802154_ADDR_MODE_SHORT:
            ESP_LOGI(TAG, "Destination address: 16-bit short");
            break;
        case IEEE802154_ADDR_MODE_EXTENDED:
            ESP_LOGI(TAG, "Destination address: 64-bit extended");
            break;
        default:
            ESP_LOGW(TAG, "Destination address: Reserved (0x%x)", fcf->destAddrMode);
            break;
    }

    // Check Source Addressing Mode
    switch (fcf->srcAddrMode) {
        case IEEE802154_ADDR_MODE_NONE:
            ESP_LOGI(TAG, "Source address: None");
            break;
        case IEEE802154_ADDR_MODE_SHORT:
            ESP_LOGI(TAG, "Source address: 16-bit short");
            break;
        case IEEE802154_ADDR_MODE_EXTENDED:
            ESP_LOGI(TAG, "Source address: 64-bit extended");
            break;
        default:
            ESP_LOGW(TAG, "Source address: Reserved (0x%x)", fcf->srcAddrMode);
            break;
    }

    // Check Frame Version
    switch (fcf->frameVersion) {
        case IEEE802154_VERSION_2003:
            ESP_LOGI(TAG, "Frame version: IEEE 802.15.4-2003");
            break;
        case IEEE802154_VERSION_2006:
            ESP_LOGI(TAG, "Frame version: IEEE 802.15.4-2006");
            break;
        case IEEE802154_VERSION_RESERVED1:
        case IEEE802154_VERSION_RESERVED2:
            ESP_LOGW(TAG, "Frame version: Reserved (0x%x)", fcf->frameVersion);
            break;
        default:
            ESP_LOGW(TAG, "Frame version: Unknown (0x%x)", fcf->frameVersion);
            break;
    }

    // Check 1-bit fields (log both 1 and 0 states)
    if (fcf->securityEnabled) {
        ESP_LOGI(TAG, "Security: Enabled");
    } else {
        ESP_LOGI(TAG, "Security: Disabled");
    }

    if (fcf->framePending) {
        ESP_LOGI(TAG, "Frame pending: Yes");
    } else {
        ESP_LOGI(TAG, "Frame pending: No");
    }

    if (fcf->ackRequest) {
        ESP_LOGI(TAG, "Acknowledgment: Requested");
    } else {
        ESP_LOGI(TAG, "Acknowledgment: Not requested");
    }

    if (fcf->panIdCompression) {
        ESP_LOGI(TAG, "PAN ID compression: Enabled");
    } else {
        ESP_LOGI(TAG, "PAN ID compression: Disabled");
    }

    if (fcf->sequenceNumberSuppression) {
        ESP_LOGI(TAG, "Sequence number suppression: Enabled");
    } else {
        ESP_LOGI(TAG, "Sequence number suppression: Disabled");
    }

    if (fcf->informationElementsPresent) {
        ESP_LOGI(TAG, "Information elements: Present");
    } else {
        ESP_LOGI(TAG, "Information elements: Not present");
    }

    if (fcf->reserved) {
        ESP_LOGI(TAG, "Reserved bit: Set");
    } else {
        ESP_LOGI(TAG, "Reserved bit: Cleared");
    }
}

// Parse IEEE 802.15.4 frame
bool ieee802154_frame_parse(const uint8_t *data, size_t len, ieee802154_frame_t *frame, bool verbose) {
    if (!data || !frame || len < IEEE802154_FCF_SIZE + IEEE802154_RSSI_LQI_SIZE) {
        ESP_LOGE(TAG, "Invalid input or frame too short");
        return false;
    }

    size_t offset = 0;

    // Parse FCF
    if (offset + IEEE802154_FCF_SIZE > len) {
        ESP_LOGE(TAG, "Frame too short for FCF");
        return false;
    }
    memcpy(&frame->fcf, data + offset, IEEE802154_FCF_SIZE);
    offset += IEEE802154_FCF_SIZE;

    // Process FCF for debugging
    process_fcf(&frame->fcf, verbose);

    // Parse Sequence Number
    if (!frame->fcf.sequenceNumberSuppression) {
        if (offset + 1 > len) {
            ESP_LOGE(TAG, "Frame too short for Sequence Number");
            return false;
        }
        frame->sequenceNumber = data[offset];
        if (verbose) {
            ESP_LOGI(TAG, "Sequence number: 0x%02x", frame->sequenceNumber);
        }
        offset += 1;
    } else {
        frame->sequenceNumber = 0; // Not present
        if (verbose) {
            ESP_LOGI(TAG, "Sequence number: Suppressed");
        }
    }

    // Parse Destination PAN ID
    bool hasDestAddr = (frame->fcf.destAddrMode != IEEE802154_ADDR_MODE_NONE);
    bool hasSrcAddr = (frame->fcf.srcAddrMode != IEEE802154_ADDR_MODE_NONE);
    if (hasDestAddr) {
        if (offset + IEEE802154_PAN_ID_LEN > len) {
            ESP_LOGE(TAG, "Frame too short for Destination PAN ID");
            return false;
        }
        frame->destPanId = (data[offset + 1] << 8) | data[offset]; // Little-endian
        if (verbose) {
            ESP_LOGI(TAG, "Destination PAN ID: 0x%04x", frame->destPanId);
        }
        offset += IEEE802154_PAN_ID_LEN;
    } else {
        frame->destPanId = 0;
        if (verbose) {
            ESP_LOGI(TAG, "Destination PAN ID: None");
        }
    }

    // Parse Destination Address
    size_t destAddrLen = 0;
    if (frame->fcf.destAddrMode == IEEE802154_ADDR_MODE_SHORT) {
        destAddrLen = 2;
    } else if (frame->fcf.destAddrMode == IEEE802154_ADDR_MODE_EXTENDED) {
        destAddrLen = 8;
    }
    frame->destAddrLen = destAddrLen; // Set destination address length
    if (destAddrLen > 0) {
        if (offset + destAddrLen > len) {
            ESP_LOGE(TAG, "Frame too short for Destination Address");
            return false;
        }
        memcpy(frame->destAddress, data + offset, destAddrLen);
        if (verbose) {
            if (destAddrLen == 2) {
                uint16_t addr = (frame->destAddress[1] << 8) | frame->destAddress[0];
                ESP_LOGI(TAG, "Destination address: 0x%04x (short)", addr);
            } else {
                ESP_LOGI(TAG, "Destination address: %02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x (extended)",
                         frame->destAddress[0], frame->destAddress[1], frame->destAddress[2], frame->destAddress[3],
                         frame->destAddress[4], frame->destAddress[5], frame->destAddress[6], frame->destAddress[7]);
            }
        }
        offset += destAddrLen;
    } else {
        memset(frame->destAddress, 0, IEEE802154_MAX_ADDR_LEN);
    }

    // Parse Source PAN ID
    if (hasSrcAddr && !frame->fcf.panIdCompression) {
        if (offset + IEEE802154_PAN_ID_LEN > len) {
            ESP_LOGE(TAG, "Frame too short for Source PAN ID");
            return false;
        }
        frame->srcPanId = (data[offset + 1] << 8) | data[offset]; // Little-endian
        if (verbose) {
            ESP_LOGI(TAG, "Source PAN ID: 0x%04x", frame->srcPanId);
        }
        offset += IEEE802154_PAN_ID_LEN;
    } else if (hasSrcAddr && frame->fcf.panIdCompression) {
        frame->srcPanId = frame->destPanId; // PAN ID compression
        if (verbose) {
            ESP_LOGI(TAG, "Source PAN ID: 0x%04x (compressed)", frame->srcPanId);
        }
    } else {
        frame->srcPanId = 0;
        if (verbose) {
            ESP_LOGI(TAG, "Source PAN ID: None");
        }
    }

    // Parse Source Address
    size_t srcAddrLen = 0;
    if (frame->fcf.srcAddrMode == IEEE802154_ADDR_MODE_SHORT) {
        srcAddrLen = 2;
    } else if (frame->fcf.srcAddrMode == IEEE802154_ADDR_MODE_EXTENDED) {
        srcAddrLen = 8;
    }
    frame->srcAddrLen = srcAddrLen; // Set source address length
    if (srcAddrLen > 0) {
        if (offset + srcAddrLen > len) {
            ESP_LOGE(TAG, "Frame too short for Source Address");
            return false;
        }
        memcpy(frame->srcAddress, data + offset, srcAddrLen);
        if (verbose) {
            if (srcAddrLen == 2) {
                uint16_t addr = (frame->srcAddress[1] << 8) | frame->srcAddress[0];
                ESP_LOGI(TAG, "Source address: 0x%04x (short)", addr);
            } else {
                ESP_LOGI(TAG, "Source address: %02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x (extended)",
                         frame->srcAddress[0], frame->srcAddress[1], frame->srcAddress[2], frame->srcAddress[3],
                         frame->srcAddress[4], frame->srcAddress[5], frame->srcAddress[6], frame->srcAddress[7]);
            }
        }
        offset += srcAddrLen;
    } else {
        memset(frame->srcAddress, 0, IEEE802154_MAX_ADDR_LEN);
    }

    // Parse RSSI/LQI (combined in 1 byte)
    if (offset + IEEE802154_RSSI_LQI_SIZE > len) {
        ESP_LOGE(TAG, "Frame too short for RSSI/LQI");
        return false;
    }
    frame->rssi_lqi = data[len - 1]; // Last byte is combined RSSI and LQI
    if (verbose) {
        ESP_LOGI(TAG, "RSSI/LQI: 0x%02x", frame->rssi_lqi);
    }

    // Parse Payload
    frame->payloadLen = len - offset - IEEE802154_RSSI_LQI_SIZE;
    frame->payload = (frame->payloadLen > 0) ? (uint8_t *)(data + offset) : NULL;
    if (verbose) {
        ESP_LOGI(TAG, "Payload length: %zu bytes", frame->payloadLen);
        if (frame->payloadLen > 0) {
            ESP_LOG_BUFFER_HEX(TAG, frame->payload, frame->payloadLen > 16 ? 16 : frame->payloadLen);
        }
    }

    return true;
}

// Build IEEE 802.15.4 frame
// Note: RSSI/LQI are added by hardware on receive, not included in transmit
size_t ieee802154_frame_build(const ieee802154_frame_t *frame, uint8_t *buffer, size_t buffer_size) {
    if (!frame || !buffer) {
        ESP_LOGE(TAG, "Invalid input");
        return 0;
    }

    ESP_LOGI(TAG, "FCF: frameType=%d, securityEnabled=%d, framePending=%d, "
             "ackRequest=%d, panIdCompression=%d, reserved=%d, "
             "sequenceNumberSuppression=%d, informationElementsPresent=%d, "
             "destAddrMode=%d, frameVersion=%d, srcAddrMode=%d",
             frame->fcf.frameType, frame->fcf.securityEnabled, frame->fcf.framePending,
             frame->fcf.ackRequest, frame->fcf.panIdCompression, frame->fcf.reserved,
             frame->fcf.sequenceNumberSuppression, frame->fcf.informationElementsPresent,
             frame->fcf.destAddrMode, frame->fcf.frameVersion, frame->fcf.srcAddrMode);    

    size_t offset = 0;

    // Write FCF
    if (offset + IEEE802154_FCF_SIZE > buffer_size) {
        ESP_LOGE(TAG, "Buffer too small for FCF");
        return 0;
    }
    memcpy(buffer + offset, &frame->fcf, IEEE802154_FCF_SIZE);
    offset += IEEE802154_FCF_SIZE;

    // Write Sequence Number
    if (!frame->fcf.sequenceNumberSuppression) {
        if (offset + 1 > buffer_size) {
            ESP_LOGE(TAG, "Buffer too small for Sequence Number");
            return 0;
        }
        buffer[offset] = frame->sequenceNumber;
        offset += 1;
    }

    // Write Destination PAN ID
    bool hasDestAddr = (frame->fcf.destAddrMode != IEEE802154_ADDR_MODE_NONE);
    bool hasSrcAddr = (frame->fcf.srcAddrMode != IEEE802154_ADDR_MODE_NONE);
    if (hasDestAddr) {
        if (offset + IEEE802154_PAN_ID_LEN > buffer_size) {
            ESP_LOGE(TAG, "Buffer too small for Destination PAN ID");
            return 0;
        }
        buffer[offset] = frame->destPanId & 0xFF;
        buffer[offset + 1] = (frame->destPanId >> 8) & 0xFF;
        offset += IEEE802154_PAN_ID_LEN;
    }

    // Write Destination Address
    size_t destAddrLen = 0;
    if (frame->fcf.destAddrMode == IEEE802154_ADDR_MODE_SHORT) {
        destAddrLen = 2;
    } else if (frame->fcf.destAddrMode == IEEE802154_ADDR_MODE_EXTENDED) {
        destAddrLen = 8;
    }
    if (destAddrLen > 0) {
        if (offset + destAddrLen > buffer_size) {
            ESP_LOGE(TAG, "Buffer too small for Destination Address");
            return 0;
        }
        memcpy(buffer + offset, frame->destAddress, destAddrLen);
        offset += destAddrLen;
    }

    // Write Source PAN ID
    if (hasSrcAddr && !frame->fcf.panIdCompression) {
        if (offset + IEEE802154_PAN_ID_LEN > buffer_size) {
            ESP_LOGE(TAG, "Buffer too small for Source PAN ID");
            return 0;
        }
        buffer[offset] = frame->srcPanId & 0xFF;
        buffer[offset + 1] = (frame->srcPanId >> 8) & 0xFF;
        offset += IEEE802154_PAN_ID_LEN;
    }

    // Write Source Address
    size_t srcAddrLen = 0;
    if (frame->fcf.srcAddrMode == IEEE802154_ADDR_MODE_SHORT) {
        srcAddrLen = 2;
    } else if (frame->fcf.srcAddrMode == IEEE802154_ADDR_MODE_EXTENDED) {
        srcAddrLen = 8;
    }
    if (srcAddrLen > 0) {
        if (offset + srcAddrLen > buffer_size) {
            ESP_LOGE(TAG, "Buffer too short for Source Address");
            return false;
        }
        memcpy(buffer + offset, frame->srcAddress, srcAddrLen);
        offset += srcAddrLen;
    }

    // Write Payload
    if (frame->payloadLen > 0 && frame->payload) {
        if (offset + frame->payloadLen > buffer_size) {
            ESP_LOGE(TAG, "Buffer too small for Payload");
            return 0;
        }
        memcpy(buffer + offset, frame->payload, frame->payloadLen);
        offset += frame->payloadLen;
    }

    ESP_LOGI(TAG, "Built frame of %zu bytes", offset);
    return offset;
}

// Debug function to convert frame type to string
const char* ieee802154_frame_type_to_str(uint8_t frameType) {
    switch (frameType) {
        case IEEE802154_FRAME_TYPE_BEACON: return "Beacon";
        case IEEE802154_FRAME_TYPE_DATA: return "Data";
        case IEEE802154_FRAME_TYPE_ACK: return "ACK";
        case IEEE802154_FRAME_TYPE_MAC_CMD: return "MAC Command";
        default: return "Reserved";
    }
}