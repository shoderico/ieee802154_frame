#include <stdio.h>
#include <string.h>

#include <unity.h>
//#include "unity_test_utils.h"
//#include "unity_test_runner.h"

#include <esp_log.h>
//#include <esp_mac.h>

#include "ieee802154_frame.h"

// Test case: Parse a valid data frame
TEST_CASE("Parse a valid data frame", "[valid]") {
    uint8_t raw_frame[] = {
        0x41, 0x88, // FCF: Data, short addresses, 2003, PAN ID compression
        0xdb,       // Sequence Number
        0xe7, 0x00, // Dest PAN ID
        0xff, 0xff, // Dest Address
        0x96, 0xf0, // Src Address
        0xc9, 0x80, 0x00, 0x00, 0x00, 0xb7, // Payload
        0xcc        // RSSI/LQI
    };
    ieee802154_frame_t frame = {0};

    bool result = ieee802154_frame_parse(raw_frame, sizeof(raw_frame), &frame, false);
    TEST_ASSERT_TRUE(result);
    TEST_ASSERT_EQUAL(IEEE802154_FRAME_TYPE_DATA, frame.fcf.frameType);
    TEST_ASSERT_EQUAL(0xdb, frame.sequenceNumber);
    TEST_ASSERT_EQUAL(0x00e7, frame.destPanId);
    
    { uint8_t expected[] = {0xff, 0xff}; TEST_ASSERT_EQUAL_UINT8_ARRAY(expected, frame.destAddress, 2); }
    TEST_ASSERT_EQUAL(0x00e7, frame.srcPanId); // Compressed
    { uint8_t expected[] = {0x96, 0xf0}; TEST_ASSERT_EQUAL_UINT8_ARRAY(expected, frame.srcAddress, 2); }
    TEST_ASSERT_EQUAL(6, frame.payloadLen);
    { uint8_t expected[] = {0xc9, 0x80, 0x00, 0x00, 0x00, 0xb7}; TEST_ASSERT_EQUAL_UINT8_ARRAY(expected, frame.payload, 6); }
    TEST_ASSERT_EQUAL(0xcc, frame.rssi_lqi);
    TEST_ASSERT_EQUAL(2, frame.destAddrLen); // Check destination address length
    TEST_ASSERT_EQUAL(2, frame.srcAddrLen);  // Check source address length
}

// Test case: Parse an invalid frame (too short)
TEST_CASE("Parse an invalid frame (too short)", "[invalid]") {
    uint8_t raw_frame[] = {0x41, 0x88}; // Too short
    ieee802154_frame_t frame = {0};

    bool result = ieee802154_frame_parse(raw_frame, sizeof(raw_frame), &frame, false);
    TEST_ASSERT_FALSE(result);
}

// Test case: Parse a frame with no addresses
TEST_CASE("Parse a frame with no addresses", "[valid]") {
    uint8_t raw_frame[] = {
        0x01, 0x00, // FCF: Data, no addresses, 2003
        0xdb,       // Sequence Number
        0xc9, 0x80, 0x00, // Payload
        0xcc        // RSSI/LQI
    };
    ieee802154_frame_t frame = {0};

    bool result = ieee802154_frame_parse(raw_frame, sizeof(raw_frame), &frame, false);
    TEST_ASSERT_TRUE(result);
    TEST_ASSERT_EQUAL(IEEE802154_FRAME_TYPE_DATA, frame.fcf.frameType);
    TEST_ASSERT_EQUAL(0xdb, frame.sequenceNumber);
    TEST_ASSERT_EQUAL(0, frame.destPanId);
    TEST_ASSERT_EQUAL(0, frame.srcPanId);
    TEST_ASSERT_EQUAL(3, frame.payloadLen);
    { uint8_t expected[] = {0xc9, 0x80, 0x00}; TEST_ASSERT_EQUAL_UINT8_ARRAY(expected, frame.payload, 3); }
    TEST_ASSERT_EQUAL(0xcc, frame.rssi_lqi);
    TEST_ASSERT_EQUAL(0, frame.destAddrLen); // Check destination address length
    TEST_ASSERT_EQUAL(0, frame.srcAddrLen);  // Check source address length
}

// Test case: Build a frame
TEST_CASE("Build a frame", "[buld]") {
    uint8_t buffer[128];
    ieee802154_frame_t tx_frame = {
        .fcf = {
            .frameType = IEEE802154_FRAME_TYPE_DATA, // 001
            .securityEnabled = 0,                    // 0
            .framePending = 0,                       // 0
            .ackRequest = 1,                         // 1
            .panIdCompression = 1,                   // 1
            .reserved = 0,                           // 0
            .sequenceNumberSuppression = 0,          // 0
            .informationElementsPresent = 0,         // 0
            .destAddrMode = IEEE802154_ADDR_MODE_SHORT, // 10
            .frameVersion = IEEE802154_VERSION_2006,    // 01
            .srcAddrMode = IEEE802154_ADDR_MODE_SHORT   // 10
        },
        .sequenceNumber = 0x02,
        .destPanId = 0x1234,
        .destAddress = {0x56, 0x78},
        .srcPanId = 0x1234,
        .srcAddress = {0x9A, 0xBC},
        .payloadLen = 3,
        .payload = (uint8_t[]){0x44, 0x55, 0x66}
    };

    size_t len = ieee802154_frame_build(&tx_frame, buffer, sizeof(buffer), true);
    TEST_ASSERT_GREATER_THAN(0, len);
    // Expected frame: FCF(2) + Seq(1) + DestPAN(2) + DestAddr(2) + SrcAddr(2) + Payload(3)
    TEST_ASSERT_EQUAL(12, len);
    uint8_t expected[] = {
        0x61, 0x98, // FCF: Data, ACK, short addresses, 2015, PAN ID compression
        0x02,       // Sequence Number
        0x34, 0x12, // Dest PAN ID
        0x56, 0x78, // Dest Address
        0x9A, 0xBC, // Src Address
        0x44, 0x55, 0x66 // Payload
    };
    ESP_LOG_BUFFER_HEX_LEVEL("DUMP", buffer, len, ESP_LOG_INFO);
    TEST_ASSERT_EQUAL_UINT8_ARRAY(expected, buffer, len);
}