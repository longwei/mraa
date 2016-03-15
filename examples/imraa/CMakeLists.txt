add_executable (imraa imraa.c)
add_executable (demo_test demo_test.c)

include_directories(${PROJECT_SOURCE_DIR}/api)
include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories(${PROJECT_SOURCE_DIR}/api/mraa)

target_link_libraries (imraa mraa json-c udev)
target_link_libraries (demo_test mraa)

set_property (TARGET imraa PROPERTY C_STANDARD 99)
