cmake_minimum_required(VERSION 3.14)

project(common_protocol LANGUAGES CXX
        DESCRIPTION "Filling-station on-wire protocol (FCU / ECU / GS shared definitions)")

# Header-only: an INTERFACE target that exposes the repo root as an include path, so
# consumers #include "framing/ethernet_header.hpp", "command/command_type.hpp", etc.
add_library(common_protocol INTERFACE)
add_library(common_protocol::common_protocol ALIAS common_protocol)

target_include_directories(common_protocol INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:include>)

target_compile_features(common_protocol INTERFACE cxx_std_17)
