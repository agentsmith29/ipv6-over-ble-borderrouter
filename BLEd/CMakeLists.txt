cmake_minimum_required(VERSION 3.2)
project(BLEd)


#set(CMAKE_CXX_STANDARD 14)

SET(PATH_LIBCONFIG ${PROJECT_SOURCE_DIR}/libconfig/lib/)
SET(PATH_BLUEZ ${PROJECT_SOURCE_DIR}/bluez/lib)
SET(LIB_PATH ${PROJECT_SOURCE_DIR}/build/lib)

if(APPLE)
 set(CMAKE_C_COMPILER "gcc")
set(CMAKE_CXX_COMPILER "g++")
 #configure_file(machine/endian.h endian.h COPYONLY)
 INCLUDE_DIRECTORIES("/usr/include/machine/")
 INCLUDE_DIRECTORIES("/usr/include")
endif()



#For die BlueZLib
INCLUDE_DIRECTORIES(${PATH_LIBCONFIG})
INCLUDE_DIRECTORIES(${PATH_BLUEZ})

FIND_LIBRARY(PROTOBUF protobuf)

#find_path(OPENNI_INCLUDE_PATH bluetooth/bluetooth.h)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)


configure_file(${PATH_BLUEZ}/bluetooth.h ${PATH_BLUEZ}/bluetooth/bluetooth.h COPYONLY)
configure_file(${PATH_BLUEZ}/hci.h ${PATH_BLUEZ}/bluetooth/hci.h COPYONLY)
configure_file(${PATH_BLUEZ}/sdp.h ${PATH_BLUEZ}/bluetooth/sdp.h COPYONLY)


# Create the bluetooth Library
add_library(bluetooth
        ${PATH_BLUEZ}/bluetooth.c
        ${PATH_BLUEZ}/bluetooth.h
        ${PATH_BLUEZ}/hci.c
        ${PATH_BLUEZ}/hci.h
        ${PATH_BLUEZ}/sdp.c
        ${PATH_BLUEZ}/sdp.h
       )


# Create the libconfig Library
add_library(libconfig++
       ${PATH_LIBCONFIG}/grammar.h
       ${PATH_LIBCONFIG}/parsectx.h
       ${PATH_LIBCONFIG}/scanctx.h
       ${PATH_LIBCONFIG}/scanner.h
       ${PATH_LIBCONFIG}/win32/stdint.h
       ${PATH_LIBCONFIG}/strbuf.h
       ${PATH_LIBCONFIG}/strvec.h
       ${PATH_LIBCONFIG}/util.h
       ${PATH_LIBCONFIG}/wincompat.h
       ${PATH_LIBCONFIG}/grammar.c
       ${PATH_LIBCONFIG}/libconfig.c
       ${PATH_LIBCONFIG}/scanctx.c
       ${PATH_LIBCONFIG}/scanner.c
       ${PATH_LIBCONFIG}/strbuf.c
       ${PATH_LIBCONFIG}/strvec.c
       ${PATH_LIBCONFIG}/util.c
       ${PATH_LIBCONFIG}/wincompat.c
        #--
       ${PATH_LIBCONFIG}/libconfigcpp.cc
       ${PATH_LIBCONFIG}/libconfig.h++
       ${PATH_LIBCONFIG}/libconfig.hh)


set_target_properties(libconfig++
        PROPERTIES LINKER_LANGUAGE CXX
        PUBLIC_HEADER "${libinc_cpp}")


add_executable(BLEd
        main.cpp
        src/HwAddress.cpp src/HwAddress.h
        src/Logger.cpp src/Logger.h
        src/Node.cpp src/Node.h
        src/NodesHandler.cpp src/NodesHandler.h
        src/BLEHelper.cpp src/BLEHelper.h
        src/InteractionInterface.cpp src/InteractionInterface.h
        src/HCICore/HCIInterface.cpp src/HCICore/HCIInterface.h
        src/Defines.h
        src/HCICore/ConnectionGenrator.cpp src/HCICore/ConnectionGenrator.h
        src/ICommand.cpp src/ICommand.h
        src/NodeMappings.cpp src/NodeMappings.h
        src/HCICore/LEScan.cpp
        src/HCICore/LECommand.cpp
        src/proto/node_message.pb.h src/proto/node_message.pb.cc
        src/ConfigHandler.cpp src/ConfigHandler.h src/UtilityFunctions.cpp src/UtilityFunctions.h)



target_link_libraries(BLEd
        ${PROTOBUF}
        bluetooth
        libconfig++
        pthread)

install(TARGETS BLEd DESTINATION /usr/bin/)
install(FILES BLEd.service DESTINATION "/etc/systemd/system/") #Copy the file, so syystemd can start up the binary
install(FILES configs/bled_config.conf DESTINATION "$ENV{HOME}/.bled_config") #Copy the file, so syystemd can start up the binary

SET(CPACK_GENERATOR "DEB")
SET(CPACK_PACKAGE_NAME "BLEd")

# Set the package version
SET(CPACK_PACKAGE_VERSION_MAJOR "1")
SET(CPACK_PACKAGE_VERSION_MINOR "0")
SET(CPACK_PACKAGE_VERSION_PATCH "11")
SET(CPACK_PACKAGE_DESCRIPTION_FILE "../README.md")
SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "BLEd - an Deamon Application for enabling the IPv6-over-BLE Router Functionality")
set(CPACK_DEBIAN_PACKAGE_DEPENDS "bluetooth (>= 5.4)")
SET(CPACK_DEBIAN_PACKAGE_MAINTAINER "Christoph Schmidt") #required
INCLUDE(CPack)
