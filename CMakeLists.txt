cmake_minimum_required(VERSION 3.21)

project(G2OWS VERSION 1.1.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
set(G2OWS_MODULE G2OWS)

add_library(${G2OWS_MODULE} SHARED)

# Install vcpkg
set(VCPKG_ROOT "${CMAKE_SOURCE_DIR}/vcpkg" CACHE PATH "Path to vcpkg")
if(NOT EXISTS "${VCPKG_ROOT}/.vcpkg-root")
    message(STATUS "Installing vcpkg...")
    execute_process(
        COMMAND git clone https://github.com/microsoft/vcpkg.git
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        RESULT_VARIABLE GIT_RESULT
    )

    if(WIN32)
        execute_process(
            COMMAND bootstrap-vcpkg.bat
            WORKING_DIRECTORY ${VCPKG_ROOT}
        )
    else()
        execute_process(
            COMMAND ./bootstrap-vcpkg.sh
            WORKING_DIRECTORY ${VCPKG_ROOT}
        )
    endif()
endif()

# Define vcpkg triplet
if(NOT DEFINED VCPKG_TARGET_TRIPLET)
    if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
        if(OUT_FILE_SUFFIX STREQUAL "x64")
            set(VCPKG_TARGET_TRIPLET "x64-windows" CACHE STRING "vcpkg triplet")
        else()
            set(VCPKG_TARGET_TRIPLET "x86-windows" CACHE STRING "vcpkg triplet")
        endif()
    elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
        if(OUT_FILE_SUFFIX STREQUAL "arm64")
            set(VCPKG_TARGET_TRIPLET "arm64-linux" CACHE STRING "vcpkg triplet")
        elseif(OUT_FILE_SUFFIX STREQUAL "arm")
            set(VCPKG_TARGET_TRIPLET "arm-linux" CACHE STRING "vcpkg triplet")
        else()
            set(VCPKG_TARGET_TRIPLET "x64-linux" CACHE STRING "vcpkg triplet")
        endif()
    endif()
endif()

# Setting up vcpkg toolchain
set(CMAKE_TOOLCHAIN_FILE "${VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake"
    CACHE STRING "vcpkg toolchain path")

# Installing OpenSSL
if(NOT DEFINED OPENSSL_ROOT_DIR)
    message(STATUS "Installing OpenSSL for triplet ${VCPKG_TARGET_TRIPLET}...")
    execute_process(
        COMMAND ${VCPKG_ROOT}/vcpkg install openssl:${VCPKG_TARGET_TRIPLET}
        WORKING_DIRECTORY ${VCPKG_ROOT}
        OUTPUT_VARIABLE VCPKG_OUTPUT
        ERROR_VARIABLE VCPKG_ERROR
        RESULT_VARIABLE VCPKG_RESULT
    )
    set(OPENSSL_ROOT_DIR "${VCPKG_ROOT}/packages/openssl_${VCPKG_TARGET_TRIPLET}")
endif()

# Defining suffix for output file
if(DEFINED OUT_FILE_SUFFIX)
	set_target_properties(${PROJECT_NAME}
			PROPERTIES 
				PREFIX ""
				SUFFIX ".${OUT_FILE_SUFFIX}${CMAKE_SHARED_LIBRARY_SUFFIX}"
	)
endif()

# Dependencies and sources
add_subdirectory(dependencies)
add_subdirectory(source)