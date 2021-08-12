cmake_minimum_required (VERSION 3.6)

SET(HERE "${CMAKE_ARGV3}")
SET(INCLUDE_PATH "${CMAKE_ARGV4}")
SET(LIBRARY_PATH "${CMAKE_ARGV5}")

message("${LIBRARY_PATH} ${INCLUDE_PATH}")
configure_file(${HERE}/pspsora/bridge/bridge.go.in ${HERE}/pspsora/bridge/bridge.go)
