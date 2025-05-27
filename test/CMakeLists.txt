idf_component_register(
    SRCS "test_frame.c"
    INCLUDE_DIRS "."
    REQUIRES esp_common ieee802154_frame unity
    WHOLE_ARCHIVE
)
