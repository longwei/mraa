add_executable (imraa imraa.c)

include_directories(${PROJECT_SOURCE_DIR}/api)
include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories(${PROJECT_SOURCE_DIR}/api/mraa)

target_link_libraries (imraa mraa json-c)

set_property (TARGET imraa PROPERTY C_STANDARD 99)
