add_executable (imraa imraa.c)
add_executable (blink_onboard_imraa blink_onboard_imraa.c)

include_directories(${PROJECT_SOURCE_DIR}/api)
include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories(${PROJECT_SOURCE_DIR}/api/mraa)

target_link_libraries (imraa mraa json-c)
target_link_libraries (blink_onboard_imraa mraa)

set_property (TARGET imraa PROPERTY C_STANDARD 99)
