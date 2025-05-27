# IEEE 802.15.4 Frame Parser Component

This component provides utilities to parse and build IEEE 802.15.4 MAC frames, compatible with ESP-IDF's IEEE 802.15.4 stack.

## Features
- Parse received frames into a structured format (`ieee802154_frame_parse`).
- Build frames for transmission (`ieee802154_frame_build`).
- Convert frame type to string (`ieee802154_frame_type_to_str`).
- Support for RSSI/LQI (1 byte) appended by ESP-IDF hardware.
- Verbose logging option for debugging (controlled by `verbose` parameter in `ieee802154_frame_parse`).

## Installation

### Option 1: Using ESP-IDF Component Registry
The component is available on the [ESP-IDF Component Registry](https://components.espressif.com/). Install it using:

```bash
idf.py add-dependency shoderico/ieee802154_frame==1.0.0
```

This downloads the component to your project's `managed_components` directory.

### Option 2: Manual Installation
1. Clone or download the repository:
   ```bash
   git clone https://github.com/shoderico/ieee802154_frame.git
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

void handle_received_frame(uint8_t *data, size_t len) {
    ieee802154_frame_t frame = {0};
    // Parse with verbose logging
    if (ieee802154_frame_parse(data, len, &frame, true)) {
        ESP_LOGI("APP", "Frame type: %s, RSSI/LQI: 0x%02x",
                 ieee802154_frame_type_to_str(frame.fcf.frameType), frame.rssi_lqi);
    }
    // Parse without verbose logging (faster)
    if (ieee802154_frame_parse(data, len, &frame, false)) {
        // Process frame
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
idf.py build flash monitor
```

The tests are defined in `test/test_frame.c` and executed via the `test_runner` project, ensuring the component's core functions work as expected.

## Notes
- **RSSI/LQI**: Stored as a single byte (`rssi_lqi`). The format is not fully specified in ESP-IDF; consult Espressif documentation for details.
- **Payload**: The `frame.payload` pointer references input data; ensure data remains valid during use.
- **Verbose Logging**: Set `verbose = false` in `ieee802154_frame_parse` to suppress `ESP_LOGI` outputs for better performance.
- **Dependencies**: Requires ESP-IDF v5.0 or later and the `esp_common` component.
- **Testing**: Tests require an ESP32 or compatible device for execution.

## Contributing
Contributions are welcome! Please open an issue or submit a pull request at [GitHub](https://github.com/shoderico/ieee802154_frame).

## License
MIT License. See [LICENSE](LICENSE) for details.

## Changelog
See [CHANGELOG.md](CHANGELOG.md) for version history.