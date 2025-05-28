# IEEE 802.15.4 Frame Parser Component

This component provides utilities to parse and build IEEE 802.15.4 MAC frames, compatible with ESP-IDF's IEEE 802.15.4 stack.

## Features
- Parse received frames into a structured format (`ieee802154_frame_parse`).
- Build frames for transmission with a length byte at the start and 0x00 at the end (`ieee802154_frame_build`).
- Convert frame type to string (`ieee802154_frame_type_to_str`).
- Verbose logging option for debugging (controlled by `verbose` parameter in `ieee802154_frame_parse` and `ieee802154_frame_build`).

## Installation

### Option 1: Using ESP-IDF Component Registry
The component is available on the [ESP-IDF Component Registry](https://components.espressif.com/). Install it using:

```bash
idf.py add-dependency shoderiko/ieee802154_frame==1.0.0
```

This downloads the component to your project's `managed_components` directory.

### Option 2: Manual Installation
1. Clone or download the repository:
   ```bash
   git clone https://github.com/shoderiko/ieee802154_frame.git
   ```
2. Copy the `ieee802154_frame` folder to your project's `components` directory:
   ```bash
   cp -r ieee802154_frame /path/to/your_project/components/
   ```
3. Update your project's `CMakeLists.txt` to include the component:
   ```cmake
   set(EXTRA_COMPONENT_DIRS components/ieee802154_frame)
   ```
4. Build your project:
   ```bash
   idf.py build
   ```

## Usage
```c
#include "ieee802154_frame.h"

void handle_received_frame(uint8_t *data) {
    ieee802154_frame_t frame = {0};
    // Parse with verbose logging
    if (ieee802154_frame_parse(data, &frame, true)) {
        ESP_LOGI("App", "Frame type: %s",
                 ieee802154_frame_type_to_str(frame.fcf.frameType));
    }
    // Parse without verbose logging (faster)
    if (ieee802154_frame_parse(data, &frame, false)) {
        // Process frame
    }

    // Build a frame
    uint8_t buffer[128];
    ieee802154_frame_t tx_frame = {
        .fcf = {
            .frameType = IEEE802154_FRAME_TYPE_DATA,
            .ackRequest = 1,
            .panIdCompression = 1,
            .destAddrMode = IEEE802154_ADDR_MODE_SHORT,
            .frameVersion = IEEE802154_VERSION_2006,
            .srcAddrMode = IEEE802154_ADDR_MODE_SHORT
        },
        .sequenceNumber = 0x01,
        .destPanId = 0x1234,
        .destAddress = {0x56, 0x78},
        .srcPanId = 0x1234,
        .srcAddress = {0x9A, 0xBC},
        .payloadLen = 3,
        .payload = (uint8_t[]){0x44, 0x55, 0x66}
    };
    size_t len = ieee802154_frame_build(&tx_frame, buffer, true);
    if (len > 0) {
        // Transmit buffer
    }
}
```

## Examples
Try the example projects to explore the component's functionality:
- **Simple Parse**:
  ```bash
  cd examples/simple_parse
  idf.py build flash monitor
  ```
  Demonstrates basic frame parsing with verbose logging.
- **Advanced Usage**:
  ```bash
  cd examples/advanced_usage
  idf.py build flash monitor
  ```
  Shows advanced frame building and parsing, including custom frame creation.

## Tests
Run the Unity tests to verify the component's functionality:
```bash
cd test_runner
idf.py build flash test
```

The tests are defined in `test/test_frame.c` and executed via the `test_runner` project, ensuring the component's core functions work as expected.

## Notes
- **Frame Format**: Frames have a length byte at the start (total bytes including trailing 0x00) and a 0x00 byte at the end, matching the format required by `esp_ieee802154_transmit`.
- **Buffer Size**: The caller is responsible for ensuring the output buffer in `ieee802154_frame_build` is sufficiently large (e.g., 128 bytes). No size checks are performed.
- **Payload**: The `frame.payload` pointer in `ieee802154_frame_t` references input data; ensure data remains valid during use.
- **Verbose Logging**: Set `verbose = false` in `ieee802154_frame_parse` and `ieee802154_frame_build` to suppress `ESP_LOGI` outputs for better performance.
- **Error Handling**: Minimal error checking is performed for performance. Invalid inputs or insufficient data result in `false` (parse) or `0` (build) without logging.
- **Dependencies**: Requires ESP-IDF v5.0 or later and the `esp_common` component.
- **Testing**: Tests require an ESP32 or compatible device for execution.

## Contributing
Contributions are welcome! Please open an issue or submit a pull request at [GitHub](https://github.com/shoderiko/ieee802154_frame).

## License
MIT License. See [LICENSE](LICENSE) for details.

## Changelog
See [CHANGELOG.md](CHANGELOG.md) for version history.