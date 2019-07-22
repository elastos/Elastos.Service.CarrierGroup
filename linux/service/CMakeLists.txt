cmake_minimum_required(VERSION 3.5)
project(testService)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")

set(SOURCE_FILES main.cpp testService.cpp testService.h common/json.hpp)
set(CARRIER_LIB_INCLUDE lib/)
INCLUDE_DIRECTORIES(${CARRIER_LIB_INCLUDE})
add_executable(testService ${SOURCE_FILES})

TARGET_LINK_LIBRARIES(testService pthread sqlite
        "${PROJECT_SOURCE_DIR}/lib/x86_64/libcrystal.so"
        "${PROJECT_SOURCE_DIR}/lib/x86_64/libelacarrier.so"
        "${PROJECT_SOURCE_DIR}/lib/x86_64/libelafiletrans.so"
        "${PROJECT_SOURCE_DIR}/lib/x86_64/libelasession.so")
