#include <esp_log.h>
#include <esp_mac.h>
#include "ieee802154_frame.h"

static const char *TAG = "EXAMPLE";

void app_main(void) {
    // Parse an example frame
    uint8_t raw_frame[] = {
        0x41, 0x88, 0xdc, 0xe7, 0x00, 0xff, 0xff, 0x96, 0xf0,
        0xc9, 0x80, 0x00, 0x00, 0x00, 0xb7, 0xcc
    };
    ieee802154_frame_t frame = {0};

    if (ieee802154_frame_parse(raw_frame, sizeof(raw_frame), &frame, true)) {
        ESP_LOGI(TAG, "Parsed frame: type=%s, RSSI/LQI=0x%02x",
                 ieee802154_frame_type_to_str(frame.fcf.frameType), frame.rssi_lqi);
    } else {
        ESP_LOGE(TAG, "Failed to parse frame");
    }

    // Build a custom frame
    uint8_t buffer[128];
    ieee802154_frame_t tx_frame = {
        .fcf = {
            .frameType = IEEE802154_FRAME_TYPE_DATA,
            .ackRequest = 1,
            .destAddrMode = IEEE802154_ADDR_MODE_SHORT,
            .srcAddrMode = IEEE802154_ADDR_MODE_SHORT,
            .frameVersion = IEEE802154_VERSION_2006,
            .panIdCompression = 1
        },
        .sequenceNumber = 0x02,
        .destPanId = 0x1234,
        .destAddress = {0x56, 0x78},
        .srcPanId = 0x1234,
        .srcAddress = {0x9A, 0xBC},
        .payloadLen = 3,
        .payload = (uint8_t[]){0x44, 0x55, 0x66}
    };

    size_t len = ieee802154_frame_build(&tx_frame, buffer, sizeof(buffer));
    if (len > 0) {
        ESP_LOGI(TAG, "Built frame of %zu bytes", len);
        ESP_LOG_BUFFER_HEX(TAG, buffer, len);
    } else {
        ESP_LOGE(TAG, "Failed to build frame");
    }
}