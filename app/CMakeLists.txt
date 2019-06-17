# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.

cmake_minimum_required(VERSION 3.4.1)
set (CMAKE_CXX_STANDARD 14)
#add_definitions( -Werror )

file( GLOB JNI_Source
      "${PROJECT_SOURCE_DIR}/src/main/jni"
      "*.cpp" )
file( GLOB JNI_Include
      "${PROJECT_SOURCE_DIR}/src/main/jni" )

file( GLOB_RECURSE Robot_Source
      "${PROJECT_SOURCE_DIR}/src/main/cpp/"
      "*/*.cpp" )

file( GLOB Robot_Include
      "${PROJECT_SOURCE_DIR}/src/main/cpp" )

file( GLOB Carrier_Include
      "${PROJECT_SOURCE_DIR}/src/main/jniLibs/include" )

add_library( chatrobot SHARED
             "${JNI_Source}"
             "${Robot_Source}")

include_directories( "${JNI_Include}"
                     "${Robot_Include}"
                    "${Carrier_Include}")
target_include_directories(chatrobot PUBLIC "src/main/jniLibs/include")
find_library( # Sets the name of the path variable.
              log-lib

              # Specifies the name of the NDK library that
              # you want CMake to locate.
              log )

# Specifies libraries CMake should link to your target library. You
# can link multiple libraries, such as libraries you define in this
# build script, prebuilt third-party libraries, or system libraries.

target_link_libraries( # Specifies the target library.
                       chatrobot

                       # Links the target library to the log library
                       # included in the NDK.
                       ${log-lib} )
target_link_libraries(chatrobot
                      "${PROJECT_SOURCE_DIR}/src/main/jniLibs/${ANDROID_ABI}/libcrystal.so"
                      "${PROJECT_SOURCE_DIR}/src/main/jniLibs/${ANDROID_ABI}/libelacarrier.so"
                      "${PROJECT_SOURCE_DIR}/src/main/jniLibs/${ANDROID_ABI}/libelafiletrans.so"
                      "${PROJECT_SOURCE_DIR}/src/main/jniLibs/${ANDROID_ABI}/libelasession.so")


