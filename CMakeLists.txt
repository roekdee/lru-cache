cmake_minimum_required(VERSION 3.14)

project(lru_cache LANGUAGES CXX VERSION 1.0.0)

# Header-only INTERFACE library target.
add_library(lru_cache INTERFACE)
add_library(lru::cache ALIAS lru_cache)

target_include_directories(lru_cache
    INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

target_compile_features(lru_cache INTERFACE cxx_std_17)

# Allow consumers to build without tests when used as a subproject.
option(LRU_CACHE_BUILD_TESTS "Build the lru_cache test suite" ON)

if(LRU_CACHE_BUILD_TESTS AND CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    enable_testing()

    include(FetchContent)
    FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
    )
    # Prevent GoogleTest from overriding compiler/linker settings on Windows.
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)

    add_executable(lru_cache_tests tests/lru_cache_test.cpp)
    target_link_libraries(lru_cache_tests
        PRIVATE
            lru::cache
            GTest::gtest_main
    )
    target_compile_features(lru_cache_tests PRIVATE cxx_std_17)

    if(MSVC)
        target_compile_options(lru_cache_tests PRIVATE /W4)
    else()
        target_compile_options(lru_cache_tests PRIVATE -Wall -Wextra)
    endif()

    include(GoogleTest)
    gtest_discover_tests(lru_cache_tests)
endif()
