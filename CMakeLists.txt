cmake_minimum_required(VERSION 3.5)

project(fbx_synthesizer)

set(CMAKE_BUILD_TYPE DEBUG)

# Check C++11 or C++0x support
include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-std=c++17" COMPILER_SUPPORTS_CXX11)
CHECK_CXX_COMPILER_FLAG("-std=c++0x" COMPILER_SUPPORTS_CXX0X)
if(COMPILER_SUPPORTS_CXX11)
   set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")
   add_definitions(-DCOMPILEDWITHC11)
   message(STATUS "Using flag -std=c++17.")
elseif(COMPILER_SUPPORTS_CXX0X)
   set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
   add_definitions(-DCOMPILEDWITHC0X)
   message(STATUS "Using flag -std=c++0x.")
else()
   message(FATAL_ERROR "The compiler ${CMAKE_CXX_COMPILER} has no C++17 support. Please use a different C++ compiler.")
endif()

set(FBXSDK_INCLUDE_DIRS ${PROJECT_SOURCE_DIR}/fbxsdk/include)
set(FBXSDK_LIB_DIRS ${PROJECT_SOURCE_DIR}/fbxsdk/lib/gcc/x64)
set(FBXSDK_DYNAMIC_DEBUG_LIB_DIRS ${FBXSDK_LIB_DIRS}/debug/libfbxsdk.so)
set(FBXSDK_DYNAMIC_RELEASE_LIB_DIRS ${FBXSDK_LIB_DIRS}/release/libfbxsdk.so)

add_executable(fbx_synthesizer 
    main.cpp
    src/common.cpp
    src/print.cpp)

target_include_directories(fbx_synthesizer
    PUBLIC
    ${FBXSDK_INCLUDE_DIRS}
    PUBLIC
    ${PROJECT_SOURCE_DIR}/include
)

target_link_libraries(fbx_synthesizer debug
    ${FBXSDK_DYNAMIC_DEBUG_LIB_DIRS}
    z
    xml2
)

target_link_libraries(fbx_synthesizer optimized 
    ${FBXSDK_DYNAMIC_RELEASE_LIB_DIRS}
    z
    xml2
)

# preprocessing shit!
add_executable(preprocessing 
    preprocessing.cpp
    src/common.cpp
    src/print.cpp)

target_include_directories(preprocessing
    PUBLIC
    ${FBXSDK_INCLUDE_DIRS}
    PUBLIC
    ${PROJECT_SOURCE_DIR}/include
)

target_link_libraries(preprocessing debug
    ${FBXSDK_DYNAMIC_DEBUG_LIB_DIRS}
    z
    xml2
)

target_link_libraries(preprocessing optimized 
    ${FBXSDK_DYNAMIC_RELEASE_LIB_DIRS}
    z
    xml2
)