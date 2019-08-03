cmake_minimum_required(VERSION 3.5)
project(carrierService)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")
set(SOURCE_FILES main.cpp carrierService.cpp carrierService.h
        ../common/Log.hpp
        ../common/Log.cpp
        ../common/json.hpp
        ../common/CommonVar.h
        source/Command/ChatRobotCmd.cpp
        source/Command/ChatRobotCmd.hpp
        source/DataBase/DatabaseProxy.h
        source/DataBase/DatabaseProxy.cpp
        source/DataBase/MemberInfo.h
        source/DataBase/MemberInfo.cpp
        source/DataBase/MessageInfo.h
        source/DataBase/MessageInfo.cpp
        source/ThirdParty/CompatibleFileSystem.hpp
        source/ThirdParty/ghc-filesystem.hpp
        source/CarrierRobot.h
        source/CarrierRobot.cpp
        source/CarrierConfig.h
        source/CarrierConfig.cpp
        source/ErrCode.cpp
        source/ErrCode.h
        source/Node.cpp
        source/Node.h)
set(CARRIER_LIB_INCLUDE lib/include/)
INCLUDE_DIRECTORIES(${CARRIER_LIB_INCLUDE})
INCLUDE_DIRECTORIES(source/)
INCLUDE_DIRECTORIES(../common/)

LINK_DIRECTORIES("source/")
add_executable(carrierService ${SOURCE_FILES})
#TARGET_LINK_LIBRARIES(carrierService pthread sqlite3 crystal elacarrier elafiletrans elasession)
TARGET_LINK_LIBRARIES(carrierService pthread sqlite3
        "${PROJECT_SOURCE_DIR}/lib/x86_64/libcrystal.so"
        "${PROJECT_SOURCE_DIR}/lib/x86_64/libelacarrier.so"
        "${PROJECT_SOURCE_DIR}/lib/x86_64/libelafiletrans.so"
        "${PROJECT_SOURCE_DIR}/lib/x86_64/libelasession.so")
