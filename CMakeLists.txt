##
# 
# recommended:
# - mkdir build
# - cd build 
# - cmake -G Ninja ..
# - ninja
#####################
#
# cmake -Bbuild -H. -GNinja
#

cmake_minimum_required(VERSION 3.9)
project(QTimeSheet VERSION 0.1)
message(STATUS "--- root ----")
include(cmake/PCH.cmake)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_BUILD_TYPE Debug)


set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

####

add_subdirectory(src)

