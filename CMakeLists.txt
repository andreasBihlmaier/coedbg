cmake_minimum_required(VERSION 3.10)
project(coedbg)

find_package(Wireshark REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_search_module(GLIB REQUIRED glib-2.0)
pkg_search_module(GMODULE REQUIRED gmodule-2.0)
list(APPEND CMAKE_MODULE_PATH ${Wireshark_DIR}/../../../cmake/modules)  # This assumes ${Wireshark_DIR}/../../.. is the Wireshark source code directory, cf. README.md
find_package(PCAP REQUIRED)
find_package(Boost COMPONENTS program_options REQUIRED)

option(LIBLIBWIRESHARK_DIR "Install dir of liblibwireshark")
if (NOT LIBLIBWIRESHARK_DIR)
  message(FATAL_ERROR "LIBLIBWIRESHARK_DIR not set")
endif (NOT LIBLIBWIRESHARK_DIR)


set(CMAKE_C_FLAGS "-Wall -Wextra -Werror")
set(CMAKE_C_STANDARD 99)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS "-Wall -Wextra -Werror")

if (NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "RelWithDebInfo" CACHE STRING "Choose the type of build." FORCE)
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif (NOT CMAKE_BUILD_TYPE)


include_directories(
  include
  ${Wireshark_INCLUDE_DIR}
  ${GLIB_INCLUDE_DIRS}
  ${GMODULE_INCLUDE_DIRS}
  ${PCAP_INCLUDE_DIRS}
  ${LIBLIBWIRESHARK_DIR}/include
)

link_directories(
  ${Wireshark_LIBRARY_DIR}
  ${GLIB_LIBRARY_DIRS}
  ${GMODULE_LIBRARY_DIRS}
  ${PCAP_LIBRARY_DIRS}
  ${LIBLIBWIRESHARK_DIR}/lib
)

add_library(coe_dbg SHARED
  src/coe_debugger.cpp
  src/coe_field.cpp
  src/coe_packet.cpp
  src/esi_parser.cpp
  src/od_base_type.cpp
  src/od.cpp
  src/od_data_type.cpp
  src/od_entry.cpp
  src/od_object.cpp
)
target_link_libraries(coe_dbg
  liblibwireshark
  ${Wireshark_LIBRARY}
  ${PCAP_LIBRARIES}
)


add_executable(coedbg
  src/main.cpp
)
target_link_libraries(coedbg
  coe_dbg
  ${Boost_PROGRAM_OPTIONS_LIBRARY}
)


#TODO add_subdirectory(tests)