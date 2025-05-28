#include <esp_log.h>
#include "ieee802154_frame.h"

static const char *TAG = "EXAMPLE";

void app_main(void) {
    uint8_t raw_frame[] = {
        0x11,       // Length (17 bytes)
        0x41, 0x88, // FCF
        0xdb,       // Sequence No.
        0xe7, 0x00, // Dest PAN ID
        0xff, 0xff, // Dest Address
        0x96, 0xf0, // Src Address
        0xc9, 0x80, 0x00, 0x00, 0x00, 0xb7, // Payload
        0x00        // Trailing 0x00
    };
    ieee802154_frame_t frame = {0};

    if (ieee802154_frame_parse(raw_frame, &frame, true)) {
        ESP_LOGI(TAG, "Frame parsed successfully, type: %s, RSSI/LQI: 0x%02x",
                 ieee802154_frame_type_to_str(frame.fcf.frameType), frame.rssi_lqi);
    } else {
        ESP_LOGE(TAG, "Failed to parse frame");
    }
}