add_library(
  Routing STATIC
  src/router.cpp
  src/rabbit_mq_client.cpp
  src/session.cpp
  src/session_read.cpp
  src/session_write.cpp
  src/util.cpp
  src/log.cpp
  src/otel_init.cpp)

target_compile_options(Routing PRIVATE -Wall -Werror -Wextra)

target_include_directories(Routing PUBLIC ${Boost_INCLUDE_DIRS}
                                          ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(Routing PRIVATE amqpcpp Boost::boost spdlog::spdlog
                                      opentelemetry-cpp::opentelemetry-cpp)
