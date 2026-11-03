cmake_minimum_required(VERSION 3.15)
project(picOverlay LANGUAGES CXX)

set(CMAKE_BUILD_TYPE Release)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(WINDOWS_RUB_INCLUDE "" CACHE PATH "Path to windowsRub include directory")
if(NOT WINDOWS_RUB_INCLUDE)
    message(FATAL_ERROR "windowsRub not configured. Set -DWINDOWS_RUB_INCLUDE=...")
endif()

find_package(cxxopts CONFIG QUIET)

if(cxxopts_FOUND)
    get_target_property(CXXOPTS_INCLUDE_DIRS cxxopts::cxxopts INTERFACE_INCLUDE_DIRECTORIES)
else()
    message(STATUS "cxxopts not found automatically. Set CXXOPTS_ROOT manually.")
    set(CXXOPTS_ROOT "" CACHE PATH "Path to cxxopts include directory")
    if(NOT CXXOPTS_ROOT)
        message(FATAL_ERROR "cxxopts not configured. Set -DCXXOPTS_ROOT=...")
    endif()
    set(CXXOPTS_INCLUDE_DIRS ${CXXOPTS_ROOT}/include)
endif()

find_package(PNG QUIET)
find_package(ZLIB QUIET)

if(PNG_FOUND)
    set(PNG_INCLUDE_DIRS ${PNG_INCLUDE_DIRS})
    set(PNG_LIBRARIES    ${PNG_LIBRARIES})
else()
    message(STATUS "PNG not found automatically. Set PNG_INCLUDE_DIR and PNG_LIBRARY manually.")
    set(PNG_INCLUDE_DIR "" CACHE PATH "Path to libpng include directory.")
    set(PNG_LIBRARY     "" CACHE FILEPATH "Path to libpng library file.")
    if(NOT PNG_INCLUDE_DIR OR NOT PNG_LIBRARY)
        message(FATAL_ERROR "PNG library not configured. Please set -DPNG_INCLUDE_DIR=... -DPNG_LIBRARY=...")
    endif()
    set(PNG_INCLUDE_DIRS ${PNG_INCLUDE_DIR})
    set(PNG_LIBRARIES    ${PNG_LIBRARY})
endif()

if(ZLIB_FOUND)
    set(ZLIB_INCLUDE_DIRS ${ZLIB_INCLUDE_DIRS})
    set(ZLIB_LIBRARIES    ${ZLIB_LIBRARIES})
else()
    message(STATUS "ZLIB not found automatically. Set ZLIB_INCLUDE_DIR and ZLIB_LIBRARY manually.")
    set(ZLIB_INCLUDE_DIR "" CACHE PATH "Path to zlib include directory.")
    set(ZLIB_LIBRARY     "" CACHE FILEPATH "Path to zlib library file.")
    if(NOT ZLIB_INCLUDE_DIR OR NOT ZLIB_LIBRARY)
        message(FATAL_ERROR "ZLIB library not configured. Please set -DZLIB_INCLUDE_DIR=... -DZLIB_LIBRARY=...")
    endif()
    set(ZLIB_INCLUDE_DIRS ${ZLIB_INCLUDE_DIR})
    set(ZLIB_LIBRARIES    ${ZLIB_LIBRARY})
endif()

add_executable(picOverlay src/picOverlay.cpp src/pngs.cpp)

target_include_directories(picOverlay PRIVATE
    ${WINDOWS_RUB_INCLUDE}
    ${CXXOPTS_INCLUDE_DIRS}
    ${PNG_INCLUDE_DIRS}
    ${ZLIB_INCLUDE_DIRS}
)

target_link_libraries(picOverlay PRIVATE
    ${PNG_LIBRARIES}
    ${ZLIB_LIBRARIES}
)

if(MSVC)
    target_compile_options(picOverlay PRIVATE /utf-8 /EHsc)
    set_target_properties(picOverlay PROPERTIES
        MSVC_RUNTIME_LIBRARY "MultiThreadedDLL"
    )
endif()