cmake_minimum_required(VERSION 3.16)

project(EAE_Firmware VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(EAE_BUILD_TESTS "Build GoogleTest unit tests" ON)
option(EAE_ALLOW_FETCH_GTEST "Allow CMake to download GoogleTest if it is not installed" OFF)

add_library(eae_firmware_lib
    src/CanBus.cpp
    src/CliOptions.cpp
    src/CoolingStateMachine.cpp
    src/FirmwareController.cpp
    src/PidController.cpp
)

target_include_directories(eae_firmware_lib
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_features(eae_firmware_lib PUBLIC cxx_std_17)

if(MSVC)
    target_compile_options(eae_firmware_lib PRIVATE /W4 /permissive-)
else()
    target_compile_options(eae_firmware_lib PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_executable(eae_firmware src/main.cpp)
target_link_libraries(eae_firmware PRIVATE eae_firmware_lib)

if(EAE_BUILD_TESTS)
    include(CTest)

    find_package(GTest CONFIG QUIET)

    if(NOT GTest_FOUND AND EAE_ALLOW_FETCH_GTEST)
        include(FetchContent)
        FetchContent_Declare(
            googletest
            URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
            DOWNLOAD_EXTRACT_TIMESTAMP TRUE
        )
        set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
        FetchContent_MakeAvailable(googletest)
    endif()

    if(TARGET GTest::gtest_main)
        add_executable(eae_firmware_tests
            tests/test_can_bus.cpp
            tests/test_firmware_controller.cpp
            tests/test_pid_controller.cpp
            tests/test_state_machine.cpp
        )
        target_link_libraries(eae_firmware_tests PRIVATE eae_firmware_lib GTest::gtest_main)

        add_test(NAME eae_firmware_tests COMMAND eae_firmware_tests)
    else()
        message(WARNING "GoogleTest was not found. Install GTest with your toolchain/vcpkg, or configure with -DEAE_ALLOW_FETCH_GTEST=ON.")
    endif()
endif()
