cmake_minimum_required(VERSION 3.16)
project(constrained_ordering)

set(CMAKE_CXX_STANDARD 14)

set(SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/src)
set(INCLUDE_DIR ${CMAKE_CURRENT_LIST_DIR}/include)
file(GLOB HEADERS "${INCLUDE_DIR}/*.h")
file(GLOB SOURCES "${SOURCE_DIR}/*.cpp" "${SOURCE_DIR}/*.c")

include_directories( ${INCLUDE_DIR} ${CMAKE_INSTALL_PREFIX}/include)

add_library(${PROJECT} STATIC ${SOURCES} include/reader.h src/reader.cpp)

add_executable(constrained_ordering src/main.cpp src/graphModel.cpp include/graphModel.h include/utils.h src/utils.cpp include/reader.h src/reader.cpp ${SOURCES})