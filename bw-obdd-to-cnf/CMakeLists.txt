cmake_minimum_required(VERSION 3.17)
project(bw_obdd_to_cnf)

set(CMAKE_CXX_STANDARD 14)
set(SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/src)
set(INCLUDE_DIR ${CMAKE_CURRENT_LIST_DIR}/include)
file(GLOB HEADERS "${INCLUDE_DIR}/*.h")
file(GLOB SOURCES "${SOURCE_DIR}/*.cpp" "${SOURCE_DIR}/*.c")
file(GLOB ORDER_SOURCES "../constrained-ordering/src/graphModel.cpp" "../constrained-ordering/src/reader.cpp")

include_directories( ${INCLUDE_DIR} ${CMAKE_INSTALL_PREFIX}/include ../constrained-ordering/include)

add_library(${PROJECT} STATIC ${SOURCES} ${ORDER_SOURCES})

set(MAIN ${SOURCE_DIR}/main.cpp include/literalMap.h src/literalmap.cpp)

add_executable(bw_obdd_to_cnf ${MAIN} ${SOURCES} ${ORDER_SOURCES})