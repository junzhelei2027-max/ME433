# This is the standard pico_sdk_import.cmake file placeholder.
# If your local build cannot find the Pico SDK, copy the official pico_sdk_import.cmake
# from your installed pico-sdk/external folder into this HW10 folder.
#
# In many ME433 repositories this file already exists in the repo root, so you can also
# keep using the existing one.

if (DEFINED ENV{PICO_SDK_PATH} AND (NOT PICO_SDK_PATH))
    set(PICO_SDK_PATH $ENV{PICO_SDK_PATH})
endif()

if (NOT PICO_SDK_PATH)
    message(FATAL_ERROR "PICO_SDK_PATH is not set. Set it to your pico-sdk folder or copy the full official pico_sdk_import.cmake.")
endif()

include(${PICO_SDK_PATH}/external/pico_sdk_import.cmake)
