cmake_minimum_required(VERSION 3.27)
project(Aspion CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(amqpcpp CONFIG REQUIRED)
find_package(Boost CONFIG REQUIRED COMPONENTS system thread)
find_package(lyra CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(opentelemetry-cpp CONFIG REQUIRED)
find_package(gRPC CONFIG REQUIRED)

add_subdirectory(src/routing)
add_subdirectory(src/server)

add_subdirectory(apps)

if (BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

if (BUILD_TESTS)
    enable_testing()
    add_subdirectory(test)
endif()
