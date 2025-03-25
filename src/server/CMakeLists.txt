add_library(Server STATIC src/server.cpp)

target_compile_options(Server PRIVATE -Wall -Werror -Wextra)

target_include_directories(Server PUBLIC ${Boost_INCLUDE_DIRS}
                                         ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(
  Server
  PUBLIC Routing
         bfg::lyra
         opentelemetry-cpp::opentelemetry-cpp
         gRPC::grpc++
         gRPC::grpc++_reflection
         gRPC::grpc
         gRPC::gpr)
