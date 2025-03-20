cmake_minimum_required(VERSION 3.27)
project(Aspion CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(amqpcpp REQUIRED)
find_package(Boost REQUIRED COMPONENTS system thread)
find_package(lyra REQUIRED)

add_library(Routing STATIC router.cpp rabbit_mq_client.cpp session.cpp
                           session_read.cpp session_write.cpp util.cpp)
add_executable(Server main.cpp)
target_compile_options(Routing PRIVATE -Wall -Werror -Wextra)
target_compile_options(Server PRIVATE -Wall -Werror -Wextra)

target_include_directories(Routing PRIVATE ${Boost_INCLUDE_DIRS})
target_include_directories(Server PRIVATE ${Boost_INCLUDE_DIRS})

target_link_libraries(Routing PRIVATE amqpcpp ${Boost_LIBRARIES})
target_link_libraries(Server PRIVATE Routing)
