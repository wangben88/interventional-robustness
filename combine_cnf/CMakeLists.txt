cmake_minimum_required(VERSION 3.16)
project(combine_cnf)

set(CMAKE_CXX_STANDARD 14)
set(SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/src)
set(INCLUDE_DIR ${CMAKE_CURRENT_LIST_DIR}/include)
file(GLOB HEADERS "${INCLUDE_DIR}/*.h")
file(GLOB SOURCES "${SOURCE_DIR}/*.cpp" "${SOURCE_DIR}/*.c")
file(GLOB ORDER_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/../constrained-ordering/src/utils.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/../constrained-ordering/src/graphModel.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/../constrained-ordering/src/reader.cpp"  "${CMAKE_CURRENT_SOURCE_DIR}/../bw-obdd-to-cnf/src/logicNode.cpp")

include_directories( ${INCLUDE_DIR} ${CMAKE_INSTALL_PREFIX}/include ${CMAKE_CURRENT_SOURCE_DIR}/../constrained-ordering/include ${CMAKE_CURRENT_SOURCE_DIR}/../bw-obdd-to-cnf/include)

add_library(${PROJECT} STATIC ${SOURCES} ${ORDER_SOURCES})

set(MAIN ${SOURCE_DIR}/main.cpp)

add_executable(combine_cnf ${MAIN} ${SOURCES} ${ORDER_SOURCES})