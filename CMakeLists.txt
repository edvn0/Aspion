cmake_minimum_required(VERSION 3.27)
project(ReqRes CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(Routing STATIC router.cpp router.hpp)
add_executable(Server server.cpp session.cpp session_read.cpp session_write.cpp)

target_link_libraries(Server PRIVATE Routing)
target_compile_options(Routing PRIVATE -Wall -Werror -Wextra)
target_compile_options(Server PRIVATE -Wall -Werror -Wextra)

if (IS_DOCKER) 
    target_include_directories(Server PRIVATE /usr/local/include/lyra/include)
else()
    target_include_directories(Server PRIVATE /home/edwin/Builds/include)
    target_include_directories(Routing PRIVATE /home/edwin/Builds/include)
endif()

