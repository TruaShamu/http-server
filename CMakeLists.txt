cmake_minimum_required(VERSION 3.15)
project(HttpServer LANGUAGES CXX)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(server
    src/main.cpp
    src/net.cpp
    src/http.cpp
    src/server.cpp
    src/threadpool.cpp
)
target_compile_options(server PRIVATE -Wall -Wextra)
target_link_libraries(server PRIVATE ws2_32)

# ---- Tests ----
include(FetchContent)
FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG        v3.5.2
)
FetchContent_MakeAvailable(Catch2)

enable_testing()

add_executable(tests
    tests/test_http.cpp
    src/http.cpp        # tests need the code under test
    src/net.cpp
)
target_link_libraries(tests PRIVATE Catch2::Catch2WithMain ws2_32)
target_compile_options(tests PRIVATE -Wall -Wextra)

include(Catch)
catch_discover_tests(tests)