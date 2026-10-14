cmake_minimum_required(VERSION 3.15)
project(picOverlay LANGUAGES CXX)

set(CMAKE_BUILD_TYPE Release)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(PNG QUIET)
find_package(ZLIB QUIET)

if(NOT PNG_FOUND)
    message(STATUS "PNG not found automatically. Set PNG_INCLUDE_DIR and PNG_LIBRARY manually.")
    set(PNG_INCLUDE_DIR "" CACHE PATH "Path to libpng include directory (e.g., C:/exchanges/code/lib/libpng/lpng1659)")
    set(PNG_LIBRARY     "" CACHE FILEPATH "Path to libpng library file (e.g., .../libpng16.lib)")
endif()

if(NOT ZLIB_FOUND)
    message(STATUS "ZLIB not found automatically. Set ZLIB_INCLUDE_DIR and ZLIB_LIBRARY manually.")
    set(ZLIB_INCLUDE_DIR "" CACHE PATH "Path to zlib include directory (e.g., C:/exchanges/code/lib/libpng/zlib)")
    set(ZLIB_LIBRARY     "" CACHE FILEPATH "Path to zlib library file (e.g., .../zlib.lib)")
endif()

if(PNG_FOUND)
    set(PNG_INCLUDE_DIRS ${PNG_INCLUDE_DIRS})
    set(PNG_LIBRARIES    ${PNG_LIBRARIES})
else()
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
    if(NOT ZLIB_INCLUDE_DIR OR NOT ZLIB_LIBRARY)
        message(FATAL_ERROR "ZLIB library not configured. Please set -DZLIB_INCLUDE_DIR=... -DZLIB_LIBRARY=...")
    endif()
    set(ZLIB_INCLUDE_DIRS ${ZLIB_INCLUDE_DIR})
    set(ZLIB_LIBRARIES    ${ZLIB_LIBRARY})
endif()

add_executable(picOverlay src/picOverlay.cpp src/pngs.cpp)

target_include_directories(picOverlay PRIVATE
    ${PNG_INCLUDE_DIRS}
    ${ZLIB_INCLUDE_DIRS}
    src/include
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