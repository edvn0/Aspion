#include "log.hpp"

#include <mutex>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

namespace Log {

namespace {

auto init_logger() -> std::shared_ptr<spdlog::logger> {
  auto sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();

  sink->set_pattern("[Aspion %Y-%m-%d %H:%M:%S.%e] [%^%l%$] [%t] %v");

  auto logger = std::make_shared<spdlog::logger>("aspion", sink);
  logger->set_level(spdlog::level::debug);
  spdlog::set_default_logger(logger);
  spdlog::flush_on(spdlog::level::info);

  return logger;
}

auto get_logger() -> std::shared_ptr<spdlog::logger> {
  static std::shared_ptr<spdlog::logger> logger = init_logger();
  return logger;
}

} // namespace

void info(std::string_view msg) { get_logger()->info("{}", msg); }

void error(std::string_view msg) { get_logger()->error("{}", msg); }

} // namespace Log