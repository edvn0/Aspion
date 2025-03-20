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

add_library(Routing STATIC router.cpp rabbit_mq_client.cpp session.cpp
    session_read.cpp session_write.cpp util.cpp log.cpp otel_init.cpp)
add_executable(Server main.cpp)
target_compile_options(Routing PRIVATE -Wall -Werror -Wextra)
target_compile_options(Server PRIVATE -Wall -Werror -Wextra)

target_include_directories(Routing PRIVATE ${Boost_INCLUDE_DIRS})
target_include_directories(Server PRIVATE ${Boost_INCLUDE_DIRS})

target_link_libraries(Routing PRIVATE amqpcpp Boost::boost spdlog::spdlog opentelemetry-cpp::opentelemetry-cpp)
target_link_libraries(Server PRIVATE
    bfg::lyra
    Routing opentelemetry-cpp::opentelemetry-cpp
    gRPC::grpc++
    gRPC::grpc++_reflection
    gRPC::grpc
    gRPC::gpr
)
