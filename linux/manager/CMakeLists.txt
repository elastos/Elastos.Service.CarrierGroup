cmake_minimum_required(VERSION 3.5)
project(carrierManager)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -fpic -lpthread")

set(SOURCE_FILES main.cpp
        ../common/Log.hpp
        ../common/Log.cpp
        ../common/FileUtils.cpp
        ../common/FileUtils.hpp
        ../common/CommonVar.h
        ../common/json.hpp
        src/GroupInfo.h
        src/GroupInfo.cpp
        src/DatabaseProxy.h
        src/DatabaseProxy.cpp
        src/manager.cpp
        src/manager.h
        managerApi.cpp)
INCLUDE_DIRECTORIES(../common/)
#add_executable(carrierManager ${SOURCE_FILES})
add_library(carrierManager SHARED ${SOURCE_FILES})
TARGET_LINK_LIBRARIES(carrierManager
        pthread
        sqlite3
        )