cmake_minimum_required(VERSION 3.5)
project(testManager)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -fPIC -lpthread")

set(SOURCE_FILES main.cpp manager.cpp manager.h common/FileUtils.cpp common/FileUtils.hpp common/json.hpp common/CommonVar.h common/Log.cpp common/Log.hpp managerApi.cpp)

#add_executable(testManager ${SOURCE_FILES})
add_library(testManager SHARED ${SOURCE_FILES})
TARGET_LINK_LIBRARIES(testManager
        pthread
        )