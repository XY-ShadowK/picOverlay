cmake_minimum_required(VERSION 3.15)
set(APP_VERSION "undefined" CACHE STRING "Application version string")
if(APP_VERSION MATCHES "^v")
    project(picOverlay VERSION ${APP_VERSION} LANGUAGES CXX)
else()
    project(picOverlay LANGUAGES CXX)
endif()
message(STATUS "project version set ${APP_VERSION}")

add_compile_definitions(CMAKE_BUILD)

set(CMAKE_BUILD_TYPE Release)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(WINDOWS_RUB_INCLUDE "" CACHE PATH "Path to windowsRub include directory")
if(NOT WINDOWS_RUB_INCLUDE)
    message(FATAL_ERROR "windowsRub not configured. Set -DWINDOWS_RUB_INCLUDE=...")
endif()

set(CXXOPTS_ROOT "" CACHE PATH "Path to cxxopts root directory")
if(CXXOPTS_ROOT)
    message(STATUS "set cxxopts with CXXOPTS_ROOT = '${CXXOPTS_ROOT} .'")
    set(CXXOPTS_INCLUDE_DIRS ${CXXOPTS_ROOT}/include)
else()
    message(STATUS "CXXOPTS_ROOT not set, automatically finding cxxopts.")
    find_package(cxxopts CONFIG QUIET)
    if(cxxopts_FOUND)
        get_target_property(CXXOPTS_INCLUDE_DIRS cxxopts::cxxopts INTERFACE_INCLUDE_DIRECTORIES)
    else()
        message(FATAL_ERROR "cxxopts not configured. Set -DCXXOPTS_ROOT=...")
    endif()
endif()

set(PNG_INCLUDE_DIR "" CACHE PATH "Path to libpng include directory.")
set(PNG_LIBRARY     "" CACHE FILEPATH "Path to libpng library file.")
if(PNG_INCLUDE_DIR AND PNG_LIBRARY)
    set(PNG_INCLUDE_DIRS ${PNG_INCLUDE_DIR})
    set(PNG_LIBRARIES    ${PNG_LIBRARY})
else()
    message(STATUS "PNG_INCLUDE_DIR or PNG_LIBRARY not set, automatically finding LIBPNG.")
    find_package(PNG QUIET)
endif()
if(NOT PNG_INCLUDE_DIRS OR NOT PNG_LIBRARIES)
    message(FATAL_ERROR "LIBPNG not configured, please confirm your installation.")
endif()

set(ZLIB_INCLUDE_DIR "" CACHE PATH "Path to zlib include directory.")
set(ZLIB_LIBRARY     "" CACHE FILEPATH "Path to zlib library file.")
if(ZLIB_INCLUDE_DIR AND ZLIB_LIBRARY)
    set(ZLIB_INCLUDE_DIRS ${ZLIB_INCLUDE_DIR})
    set(ZLIB_LIBRARIES    ${ZLIB_LIBRARY})
else()
    message(STATUS "ZLIB_INCLUDE_DIR or ZLIB_LIBRARY not set, automatically finding ZLIB.")
    find_package(ZLIB QUIET)
endif()
if(NOT ZLIB_INCLUDE_DIRS OR NOT ZLIB_LIBRARIES)
    message(FATAL_ERROR "ZLIB not configured, please confirm your installation.")
endif()

add_executable(picOverlay src/picOverlay.cpp
    src/pngs.cpp
)

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set_target_properties(frameIntervalCheck PROPERTIES
        BUILD_RPATH "$ORIGIN/lib"
        INSTALL_RPATH "$ORIGIN/lib"
    )
    target_link_options(frameIntervalCheck PRIVATE
        "-Wl,--disable-new-dtags"
    )
endif()

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


if(DEFINED ENV{RUNNER_TEMP})
    set(COMPILER_TMP "$ENV{RUNNER_TEMP}/compiler_${PROJECT_NAME}" CACHE PATH "Path to compiler temp directory")
elseif(DEFINED ENV{TMP})
    set(COMPILER_TMP "$ENV{TMP}/compiler_${PROJECT_NAME}" CACHE PATH "Path to compiler temp directory")
elseif(DEFINED ENV{TEMP})
    set(COMPILER_TMP "$ENV{TEMP}/compiler_${PROJECT_NAME}" CACHE PATH "Path to compiler temp directory")
endif()
if(COMPILER_TMP)
    configure_file(
        src/compiler/version.h.in
        ${COMPILER_TMP}/version.h
        @ONLY
    )
    target_include_directories(picOverlay PRIVATE
        ${COMPILER_TMP}
    )
endif()

if(MSVC)
    target_compile_options(picOverlay PRIVATE /utf-8 /EHsc)
    set_target_properties(picOverlay PROPERTIES
        MSVC_RUNTIME_LIBRARY "MultiThreadedDLL"
    )
endif()

install(TARGETS picOverlay
    RUNTIME DESTINATION .
)