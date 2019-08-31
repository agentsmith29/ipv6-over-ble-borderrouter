# Minimal version of CMake
cmake_minimum_required (VERSION 2.6)
# Define project name
project (6lowpan_helper)

# The version number
#set (${PROJECT_NAME}_VERSION_MAJOR 0)
#set (${PROJECT_NAME}_VERSION_MINOR 0)
#set (${PROJECT_NAME}_PATCH_LEVEL 1)

#set (${PROJECT_NAME}_VERSION
#        "${${PROJECT_NAME}_VERSION_MAJOR}.${${PROJECT_NAME}_VERSION_MINOR}")


# Set output directory for binaries
#set (EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin/)

INCLUDE_DIRECTORIES(/home/developer/workspace/IPv6-BLE-Router/linux/include/)

# Set up required subdirectories
add_executable(scantest scantest.cpp)


#target_link_libraries(6lowpan_helper ${PROJECT_SOURCE_DIR}/libconfig/lib/.libs/libconfig++.a)

