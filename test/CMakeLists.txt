cmake_minimum_required(VERSION 3.27)
project(AspionTests)

find_package(doctest CONFIG REQUIRED)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(AspionTests
    src/main.cpp
    src/path_parameter_parser_tests.cpp
    src/request_response_tests.cpp
)
target_link_libraries(AspionTests PRIVATE Routing doctest::doctest)
include(CTest)
include(doctest)
doctest_discover_tests(AspionTests)
