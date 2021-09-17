cmake_minimum_required(VERSION 3.0)

message(STATUS "Prefix: ${CMAKE_INSTALL_PREFIX}")

# set directory as project name
get_filename_component(PROJECT ${CMAKE_CURRENT_LIST_DIR} NAME)
string(REPLACE " " "_" PROJECT ${PROJECT})
project(${PROJECT})

# set compiler flags
message(STATUS "Build type: '${CMAKE_BUILD_TYPE}'")
message(STATUS "Proj: '${PROJECT}'")
set(CMAKE_OPTIMIZATION "-O3")
if (CMAKE_BUILD_TYPE MATCHES Debug OR CMAKE_BUILD_TYPE MATCHES DEBUG)
    set(CMAKE_OPTIMIZATION "-O0")
else()
    set(W "-w")
endif()
set(CMAKE_C_FLAGS "${W} ${CMAKE_OPTIMIZATION}")
set(CMAKE_CXX_FLAGS "--std=c++11 ${CMAKE_C_FLAGS}")
set(CMAKE_C_FLAGS_RELEASE "-DNDEBUG")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE}")
if(CMAKE_COMPILER_IS_GNUCXX)
    message(STATUS "GCC detected, adding compile flags")
    set(CMAKE_C_FLAGS_DEBUG "-ggdb3 -Wall -Wextra -DDEBUG   \
    -Wno-format -Wno-write-strings -Wno-unused-function \
    -Wno-unused-parameter -Wno-system-headers           \
    -Wno-format-security -Wno-ignored-qualifiers")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG}")
endif(CMAKE_COMPILER_IS_GNUCXX)


# set source files
set(SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/src)
set(INCLUDE_DIR ${CMAKE_CURRENT_LIST_DIR}/include)

file(GLOB HEADERS "${INCLUDE_DIR}/*.h")
file(GLOB SOURCES "${SOURCE_DIR}/*.cc")

set(MAIN ${SOURCE_DIR}/main.cc)
list(REMOVE_ITEM SOURCES ${MAIN})

## set include directories
include_directories( ${INCLUDE_DIR} ${CMAKE_INSTALL_PREFIX}/include)
link_directories( ${CMAKE_INSTALL_PREFIX}/lib )


# create library
add_library(${PROJECT} STATIC ${SOURCES})

# create executable
add_executable(${PROJECT}-bin ${MAIN})
set_target_properties(${PROJECT}-bin PROPERTIES OUTPUT_NAME ${PROJECT})
target_link_libraries(${PROJECT}-bin ${PROJECT} pthread)

# set install locations
install(TARGETS ${PROJECT}-bin
    RUNTIME DESTINATION "${CMAKE_INSTALL_PREFIX}/bin"
)

set_target_properties(${PROJECT} PROPERTIES PUBLIC_HEADER "${HEADERS}")
install(TARGETS ${PROJECT}
    ARCHIVE DESTINATION "${CMAKE_INSTALL_PREFIX}/lib"
    LIBRARY DESTINATION "${CMAKE_INSTALL_PREFIX}/lib"
    PUBLIC_HEADER DESTINATION "${CMAKE_INSTALL_PREFIX}/include/${PROJECT}"
)


