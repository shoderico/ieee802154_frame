cmake_minimum_required(VERSION 3.16)

set(EXTRA_COMPONENT_DIRS ..)

set(TEST_COMPONENTS "ieee802154_frame" CACHE STRING "List of components to test")

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
idf_build_set_property(MINIMAL_BUILD ON)
project(test_runner)